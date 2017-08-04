#ifdef __cplusplus
extern "C" {
#endif

#ifndef LFMODEL_H__DECLARED
#define LFMODEL_H__DECLARED

#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

typedef struct {
	/* Expected values: 
	 * -N -- writer has just allocated this slot
	 * -N+i -- writer is busy writing, but i readers are attempting to read it (will fail)
	 *  0 -- empty or outdated, no valid data; can be written to, but not read from
	 *  0+i -- empty or outdated, but i readers are attempting to read it (will fail)
	 *  N -- has valid fresh data (if active points to it)
	 *  N+i -- valid data, and i readers are currently reading it
	 */
	/* atomic */ int state;
	void *data;
} LFSlot;

typedef struct LFModel {
	int max_threads, data_size;
	size_t slot_size;
	/* atomic */ unsigned seq_active;
	/* atomic */ unsigned next_free;
	char *slot_mem;
} LFModel;

/* returned pointer should be freed by caller when it is not needed anymore */
LFModel *lfmCreate(int max_threads, int data_size, const void *initial_data, void *(*alloc)(size_t size));

typedef struct {
	const void *data_src;
	void *data_dst;

	struct {
		unsigned src, dst;
	} _;
} LFLock;

void lfmReadLock(LFModel *model, LFLock *lock);
void lfmReadUnlock(LFModel *model, LFLock *lock);

void lfmModifyLock(LFModel *model, LFLock *lock);
static inline void lfmModifyRetry(LFModel *model, LFLock *lock) { lfmReadLock(model, lock); }
/* return 0 if need to retry */
int lfmModifyUnlock(LFModel *model, LFLock *lock);

#endif // ifndef LFMODEL_H__DECLARED

#if defined(LFM_RUN_TEST)
#define LFM_IMPLEMENT
#endif

#if defined(LFM_IMPLEMENT)

#if defined(__GNUC__)
#if 1
#define MEMORY_BARRIER() __atomic_thread_fence(__ATOMIC_SEQ_CST)
#define LFM_ATOMIC_FETCH(value) __atomic_load_n(&(value), __ATOMIC_SEQ_CST)
#define LFM_ATOMIC_ADD_AND_FETCH(value, add) __atomic_add_fetch(&(value), add, __ATOMIC_SEQ_CST)
#define LFM_ATOMIC_CAS(value, expect, replace) \
	__atomic_compare_exchange_n(&(value), &(expect), replace, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#else /* old */
#define MEMORY_BARRIER() __sync_synchronize()
static inline int sync_fetch_int(int *ptr) { __sync_synchronize(); return *ptr; }
static inline unsigned sync_fetch_unsigned(unsigned *ptr) { __sync_synchronize(); return *ptr; }
//#define LFM_ATOMIC_FETCH(value) sync_fetch_unsigned(&(value))
#define LFM_ATOMIC_ADD_AND_FETCH(value, add) __sync_add_and_fetch(&(value), add)
#define LFM_ATOMIC_FETCH(value) LFM_ATOMIC_ADD_AND_FETCH(value, 0)
#define LFM_ATOMIC_CAS(value, expect, new_value) __sync_val_compare_and_swap(&(value), expect, new_value)
#endif
#elif defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOMSG
#include <windows.h>
#define MEMORY_BARRIER() MemoryBarrier()
#define LFM_ATOMIC_FETCH(value) InterlockedOr(&(value), 0)
static inline long __lfm_atomic_add_fetch(long volatile *value, long add) {
	for (;;) {
		const long expect = LFM_ATOMIC_FETCH(*value);
		if (expect == InterlockedCompareExchange(value, expect + add, expect))
			return expect + add;
	}
}
#define LFM_ATOMIC_ADD_AND_FETCH(value, add) __lfm_atomic_add_fetch(&(value), add)
#define LFM_ATOMIC_CAS(value, expect, replace) (expect == InterlockedCompareExchange(&(value), replace, expect))
#endif

#define ALIGNED_SIZE(size, alignment) ((alignment) * (((size) + (alignment) - 1) / (alignment)))

static inline LFSlot *lfm_GetSlot(const LFModel *model, unsigned index) {
	return (LFSlot*)(model->slot_mem + index * model->slot_size);
}

LFModel *lfmCreate(int max_threads, int data_size, const void *initial_data, void *(*alloc)(size_t size)) {
	// It is possible that all max_threads will try to write new data, and then the
	// last one will wait for someone to free a slot
	// TODO: increase number of slot?

	const size_t data_offset = ALIGNED_SIZE(sizeof(LFSlot), 16);
	const size_t slot_size = ALIGNED_SIZE(data_offset + data_size, 16);
	const size_t slot_offset = ALIGNED_SIZE(sizeof(LFModel), 16);
	const size_t model_size = slot_offset + max_threads * slot_size;

	char *memory = (char*)alloc(model_size);
	LFModel *model = (LFModel*)memory;
	memset(model, 0, model_size);
	model->slot_size = slot_size;
	model->slot_mem = memory + slot_offset;
	for (int i = 0; i < max_threads; ++i)
		lfm_GetSlot(model, i)->data = memory + slot_offset + slot_size * i + data_offset;

	model->max_threads = max_threads;
	model->data_size = data_size;

	LFSlot *initial = lfm_GetSlot(model, model->seq_active);
	initial->state = max_threads;
	model->next_free = 1;
	if (initial_data)
		memcpy(initial->data, initial_data, data_size);
	return model;
}

#ifdef LFM_RUN_TEST
#include <stdio.h>
static inline int doAbort() { printf("exceeded max amount of retries\n"); abort(); return 1; }
#define FOREVER() \
	for (int test_bound = 0; test_bound < 16384 || doAbort(); ++test_bound)
#else
#define FOREVER() for(;;)
#endif

void lfmModifyLock(LFModel *model, LFLock *lock) {
	LFSlot *slot = NULL;
	FOREVER() {
		const unsigned slot_index = (LFM_ATOMIC_ADD_AND_FETCH(model->next_free, 1) % model->max_threads);
		slot = lfm_GetSlot(model, slot_index);
		int state = 0;
		if (LFM_ATOMIC_CAS(slot->state, state, -model->max_threads)) {
			lock->data_dst = slot->data;
			lock->_.dst = slot_index;
			lfmReadLock(model, lock);
			return;
		}
		// TODO add random wait?
	}
}

int lfmModifyUnlock(LFModel *model, LFLock *lock) {
	lfmReadUnlock(model, lock);

	LFSlot *slot_src = lfm_GetSlot(model, lock->_.src % model->max_threads);
	LFSlot *slot_dst = lfm_GetSlot(model, lock->_.dst);

	LFM_ATOMIC_ADD_AND_FETCH(slot_dst->state, model->max_threads * 2);

	const unsigned sequence = lock->_.src / model->max_threads;
	const unsigned new_active = lock->_.dst + (sequence + 1) * model->max_threads;
	unsigned old_active = lock->_.src;
	if (!LFM_ATOMIC_CAS(model->seq_active, old_active, new_active)) {
		LFM_ATOMIC_ADD_AND_FETCH(slot_dst->state, - model->max_threads * 2);
		return 0;
	}

/// FIXME at this point both old and new slots are in "occupied" state
// this might be a problem when the system is under high load
//	LFM_ATOMIC_ADD_AND_FETCH(model->sequence, 1);

	LFM_ATOMIC_ADD_AND_FETCH(slot_src->state, - model->max_threads);

	return 1;
}

void lfmReadLock(LFModel *model, LFLock *lock) {
	FOREVER() {
		const unsigned seq_active = LFM_ATOMIC_FETCH(model->seq_active);
		LFSlot *const slot = lfm_GetSlot(model, seq_active % model->max_threads);
		const int state = LFM_ATOMIC_ADD_AND_FETCH(slot->state, 1);
		if (state > model->max_threads && seq_active == LFM_ATOMIC_FETCH(model->seq_active)) {
			lock->_.src = seq_active;
			lock->data_src = slot->data;
			break;
		}
		LFM_ATOMIC_ADD_AND_FETCH(slot->state, -1);
		// TODO add random wait?
	}
}

void lfmReadUnlock(LFModel *model, LFLock *lock) {
	(void)(model);
	LFSlot *slot = lfm_GetSlot(model, lock->_.src % model->max_threads);
	LFM_ATOMIC_ADD_AND_FETCH(slot->state, -1);
}

#endif // defined(LFM_IMPLEMENT)

#if defined(LFM_RUN_TEST)

#include <stdlib.h>

#if defined(__linux__)
#include <pthread.h>
#include <alloca.h>

typedef pthread_t ThreadHandle;

static void threadStart(ThreadHandle* handle, void (*func)(void*), void *arg) {
	pthread_create(threads + i, NULL, writeLoop, info);
}

static void threadWait(ThreadHandle handle) {
	pthread_joib(handle, NULL);
}

#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define alloca _alloca

typedef HANDLE ThreadHandle;

static void threadStart(ThreadHandle* handle, void (*func)(void*), void *arg) {
	*handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, 0, 0);
}

static void threadWait(ThreadHandle handle) {
	WaitForSingleObject(handle, INFINITE);
}
#endif


struct test_info_t {
	int readers, writers;
	int cycles;
	int data_size;
	LFModel *model;
};

struct data_t {
	int sequence;
	unsigned numbers[1];
};

static int testValidate(int *last_seq, int data_size, const struct data_t *data) {
	if (*last_seq >= 0 && *last_seq > data->sequence) {
		printf("last_seq=%d data->sequence=%d\n", *last_seq, data->sequence);
		return 0;
	}

	*last_seq = data->sequence;

	unsigned seed = *last_seq;
	for (int i = 0; i < data_size; ++i) {
		//seed = seed * 1103515245 + 123451 + i;
		seed = i + *last_seq * 1000;
		if (data->numbers[i] != seed) {
			printf("data->numbers[%d] = %u != %u\n", i, data->numbers[i], seed);
			return -i-1;
		}
	}

	return 1;
}

static void testGenerate(int sequence, int data_size, struct data_t *data) {
	data->sequence = sequence;
	for (int i = 0; i < data_size; ++i)
		data->numbers[i] = sequence * 1000 + i;
}

static void *writeLoop(void *param) {
	struct test_info_t *info = (struct test_info_t*)param;
	int last_seq = -1;
	for (int i = 0; i < info->cycles; ++i) {
		LFLock lock;
		lfmModifyLock(info->model, &lock);

		FOREVER() {
			const struct data_t *src = (const struct data_t*)lock.data_src;
			struct data_t *dst = (struct data_t*)lock.data_dst;

			const int result = testValidate(&last_seq, info->data_size, src);
			if (result != 1) {
				printf("Validation failed: i=%d, last_seq=%d, err=%d\n", i, last_seq, result);
				abort();
			}

			++last_seq;

			testGenerate(last_seq, info->data_size, dst);

			if (lfmModifyUnlock(info->model, &lock))
				break;

			lfmModifyRetry(info->model, &lock);
		}

		// TODO sleep
	}

	return NULL;
}

static void *readLoop(void *param) {
	struct test_info_t *info = (struct test_info_t*)param;
	int last_seq = -1;
	for (int i = 0; i < info->cycles; ++i) {
		LFLock lock;
		lfmReadLock(info->model, &lock);

		const struct data_t *src = (const struct data_t*)lock.data_src;

		const int result = testValidate(&last_seq, info->data_size, src);
		if (result != 1) {
			printf("Validation failed: i=%d, last_seq=%d, err=%d\n", i, last_seq, result);
			abort();
		}

		lfmReadUnlock(info->model, &lock);

		// TODO sleep
	}

	return NULL;
}

static void runTest(struct test_info_t *info) {
	printf("Running test with readers=%d writers=%d cycles=%d data_size=%d...\n",
		info->readers, info->writers, info->cycles, info->data_size);
	const int num_threads = info->readers + info->writers;
	info->model = lfmCreate(num_threads, sizeof(struct data_t) + sizeof(int) * (info->data_size - 1), NULL, malloc);

	LFLock lock;
	lfmModifyLock(info->model, &lock);

	struct data_t *dst = (struct data_t*)lock.data_dst;
	testGenerate(0, info->data_size, dst);

	if (!lfmModifyUnlock(info->model, &lock)) {
		printf("WHAT\n");
		exit(-1);
	}

	ThreadHandle *threads = (ThreadHandle*)alloca(num_threads * sizeof(ThreadHandle));
	for (int i = 0; i < info->writers; ++i) threadStart(threads + i, writeLoop, info);
	for (int i = 0; i < info->readers; ++i)	threadStart(threads + info->writers + i, readLoop, info);

	///

	for (int i = 0; i < num_threads; ++i)
		threadWait(threads[i]);

	free(info->model);
	printf("DONE\n");
}

int main(int argc, char *argv[]) {
	(void)argc; (void)argv;

	struct test_info_t info;
	info.data_size = 17;
	info.readers = 4;
	info.cycles = 1024 * 1024;
	info.writers = 0;
	runTest(&info);

	info.writers = 1;
	runTest(&info);

	info.writers = 2;
	runTest(&info);

	info.writers = 3;
	runTest(&info);

	info.data_size = 256;
	runTest(&info);

	info.data_size = 2;
	info.cycles = 4 * 1024 * 1024;
	runTest(&info);

	printf("OK? ok\n");
	return 0;
}

#endif // defined(LFM_RUN_TEST)

#ifdef __cplusplus
}
#endif
