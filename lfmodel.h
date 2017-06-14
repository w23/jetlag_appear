#ifndef LFMODEL_H_
#define LFMODEL_H_

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

typedef struct {
	int max_threads, data_size;
	size_t slot_size;
	/* atomic */ unsigned sequence;
	/* atomic */ unsigned active, next_free;
	char *slot_mem;
} LFModel;

/* returned pointer should be freed by caller when it is not needed anymore */
LFModel *lfmCreate(int max_threads, int data_size, const void *initial_data, void *(*alloc)(size_t size));

typedef struct {
	const void *data_src;
	void *data_dst;

	struct {
		int src_acq, src_seq;
		unsigned src, dst;
	} _;
} LFLock;

void lfmReadLock(LFModel *model, LFLock *lock);
void lfmReadUnlock(LFModel *model, LFLock *lock);

void lfmModifyLock(LFModel *model, LFLock *lock);
void lfmModifyRetry(LFModel *model, LFLock *lock) { lfmReadLock(model, lock); }
/* return 0 if need to retry */
int lfmModifyUnlock(LFModel *model, LFLock *lock);

#define LFM_RUN_TEST

#if defined(LFM_RUN_TEST)
#define LFM_IMPLEMENT
#endif

#if defined(LFM_IMPLEMENT)

#if 1
#define MEMORY_BARRIER() __atomic_thread_fence(__ATOMIC_SEQ_CST)
#define ATOMIC_FETCH(value) __atomic_load_n(&(value), __ATOMIC_SEQ_CST)
#define ATOMIC_ADD_AND_FETCH(value, add) __atomic_add_fetch(&(value), add, __ATOMIC_SEQ_CST)
#define ATOMIC_CAS(value, expect, replace) \
	__atomic_compare_exchange_n(&(value), &(expect), replace, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#else
#define MEMORY_BARRIER() __sync_synchronize()
static inline int sync_fetch_int(int *ptr) { __sync_synchronize(); return *ptr; }
static inline unsigned sync_fetch_unsigned(unsigned *ptr) { __sync_synchronize(); return *ptr; }
//#define ATOMIC_FETCH(value) sync_fetch_unsigned(&(value))
#define ATOMIC_ADD_AND_FETCH(value, add) __sync_add_and_fetch(&(value), add)
#define ATOMIC_FETCH(value) ATOMIC_ADD_AND_FETCH(value, 0)
#define ATOMIC_CAS(value, expect, new_value) __sync_val_compare_and_swap(&(value), expect, new_value)
#endif

#define ALIGNED_SIZE(size, alignment) ((alignment) * (((size) + (alignment) - 1) / (alignment)))

static inline LFSlot *lfm_GetSlot(const LFModel *model, unsigned index) {
	return (LFSlot*)(model->slot_mem + index * model->slot_size);
}

LFModel *lfmCreate(int max_threads, int data_size, const void *initial_data, void *(*alloc)(size_t size)) {
	//max_threads += 10;
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

	LFSlot *initial = lfm_GetSlot(model, 0);
	initial->state = max_threads;
	model->next_free = 1;
	if (initial_data)
		memcpy(initial->data, initial_data, data_size);
	return model;
}

#ifdef LFM_RUN_TEST
static inline int doAbort() { abort(); return 1; }
#define FOREVER() \
	for (int test_bound = 0; test_bound < 1024 || doAbort(); ++test_bound)
#else
#define FOREVER() for(;;)
#endif

void lfmModifyLock(LFModel *model, LFLock *lock) {
	LFSlot *slot = NULL;
	FOREVER() {
		const unsigned slot_index = (ATOMIC_ADD_AND_FETCH(model->next_free, 1) % model->max_threads);
		slot = lfm_GetSlot(model, slot_index);
		int state = 0;
		if (ATOMIC_CAS(slot->state, state, -model->max_threads)) {
			lock->data_dst = slot->data;
			lock->_.dst = slot_index;
			lfmReadLock(model, lock);
			return;
		}
	}
}

int lfmModifyUnlock(LFModel *model, LFLock *lock) {
	lfmReadUnlock(model, lock);

	LFSlot *slot_src = lfm_GetSlot(model, lock->_.src);
	LFSlot *slot_dst = lfm_GetSlot(model, lock->_.dst);

	ATOMIC_ADD_AND_FETCH(model->sequence, 1);
	ATOMIC_ADD_AND_FETCH(slot_dst->state, model->max_threads * 2);

	const unsigned new_active = lock->_.dst;
	unsigned old_active = lock->_.src;
	if (!ATOMIC_CAS(model->active, old_active, new_active)) {
		ATOMIC_ADD_AND_FETCH(slot_dst->state, - model->max_threads * 2);
		return 0;
	}

/// FIXME at this point both old and new slots are in "occupied" state
// this might be a problem when the system is under high load
//	ATOMIC_ADD_AND_FETCH(model->sequence, 1);

	ATOMIC_ADD_AND_FETCH(slot_src->state, - model->max_threads);

	return 1;
}

void lfmReadLock(LFModel *model, LFLock *lock) {
	FOREVER() {
		const unsigned sequence = ATOMIC_FETCH(model->sequence);
		const unsigned active = ATOMIC_FETCH(model->active);
		LFSlot *const slot = lfm_GetSlot(model, active);
		const int state = ATOMIC_ADD_AND_FETCH(slot->state, 1);
		if (state > model->max_threads && sequence == ATOMIC_FETCH(model->sequence)) {
			lock->_.src_seq = sequence;
			lock->_.src_acq = state;
			lock->_.src = active;
			lock->data_src = slot->data;
			break;
		}
		ATOMIC_ADD_AND_FETCH(slot->state, -1);
	}
}

void lfmReadUnlock(LFModel *model, LFLock *lock) {
	(void)(model);
	LFSlot *slot = lfm_GetSlot(model, lock->_.src);
	if (ATOMIC_ADD_AND_FETCH(slot->state, -1) == -1) abort();
}

#endif // defined(LFM_IMPLEMENT)

#if defined(LFM_RUN_TEST)

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <alloca.h>

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

	pthread_t *threads = (pthread_t*)alloca(num_threads * sizeof(pthread_t));
	for (int i = 0; i < info->writers; ++i) pthread_create(threads + i, NULL, writeLoop, info);
	for (int i = 0; i < info->readers; ++i)	pthread_create(threads + info->writers + i, NULL, readLoop, info);

	///

	for (int i = 0; i < num_threads; ++i)
		pthread_join(threads[i], NULL);

	free(info->model);
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
}

#endif // defined(LFM_RUN_TEST)
#endif // ifndef LFMODEL_H_
