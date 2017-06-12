#pragma once

#include <memory.h>
#include <stdint.h>

#define LFM_N 4
#define LFM_DATA_SIZE 32768

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
	char data[LFM_DATA_SIZE];
} LFSlot;

typedef struct {
	/* atomic */ unsigned sequence;
	/* atomic */ unsigned active, next_free;
	LFSlot slot[LFM_N];
} LFModel;

void lfmInit(LFModel *model);

typedef struct {
	const void *data_old;
	void *data_new;

	struct {
		LFSlot *slot_old, *slot_new;
	} _;
} LFLock;

void lfmReadLock(LFModel *model, LFLock *lock);
void lfmReadUnlock(LFModel *model, LFLock *lock);

void lfmModifyLock(LFModel *model, LFLock *lock);
void lfmModifyRetry(LFModel *model, LFLock *lock) { lfmReadLock(model, lock); }
int lfmModifyUnlock(LFModel *model, LFLock *lock);

#if defined(LFM_IMPLEMENT)

#define MEMORY_BARRIER() __sync_synchronize()
#define ATOMIC_FETCH(value) (value); MEMORY_BARRIER()
#define ATOMIC_ADD_AND_FETCH(value, add) __sync_add_and_fetch(&(value), add)
#define ATOMIC_CAS(value, expect, new_value) __sync_val_compare_and_swap(&(value), expect, new_value)

#define LFM_SLOT_STATE_VALID LFM_N

void lfmInit(LFModel *model) {
	memset(model, 0, sizeof(*model));
	model->slot[0].state = LFM_SLOT_STATE_VALID;
	model->next_free = 1;
}

void lfmModifyLock(LFModel *model, LFLock *lock) {
	LFSlot *slot = NULL;
	for (;;) {
		const unsigned slot_index = (ATOMIC_ADD_AND_FETCH(model->next_free, 1) % LFM_N);
		slot = model->slot + slot_index;
		int state = 0;
		for (;;) {
			const int old = ATOMIC_CAS(slot->state, state, -LFM_N);
			if (old == state) {
				state = 0;
				break;
			}
			state = old;
			if (state < 0 || state >= LFM_SLOT_STATE_VALID)
				break;
		}
		if (state != 0)
			continue;
	}

	lock->data_new = slot->data;
	lock->_.slot_new = slot;

	lfmReadLock(model, lock);
}

int lfmModifyUnlock(LFModel *model, LFLock *lock) {
	lfmReadUnlock(model, lock);

	ATOMIC_ADD_AND_FETCH(model->sequence, 1);
	MEMORY_BARRIER();
	ATOMIC_ADD_AND_FETCH(lock->_.slot_new->state, LFM_N * 2);

	const unsigned new_active = lock->_.slot_new - model->slot;
	const unsigned old_active = lock->_.slot_old - model->slot;
	if (old_active != ATOMIC_CAS(model->active, old_active, new_active)) {
		ATOMIC_ADD_AND_FETCH(lock->_.slot_new->state, -LFM_N * 2);
		return 0;
	}

	return 1;
}

void lfmReadLock(LFModel *model, LFLock *lock) {
	for (;;) {
		const unsigned sequence = ATOMIC_FETCH(model->sequence);
		MEMORY_BARRIER();
		const int active = ATOMIC_FETCH(model->active);
		LFSlot *const slot = model->slot + active;
		const int state = ATOMIC_ADD_AND_FETCH(slot->state, 1);
		MEMORY_BARRIER();
		if (state < LFM_SLOT_STATE_VALID || sequence != ATOMIC_FETCH(model->sequence)) {
			ATOMIC_ADD_AND_FETCH(slot->state, -1);
			continue;
		}

		lock->_.slot_old = slot;
		lock->data_old = slot->data;
		break;
	}
}

void lfmReadUnlock(LFModel *model, LFLock *lock) {
	(void)(model);
	ATOMIC_ADD_AND_FETCH(lock->_.slot_old->state, -1);
}

#endif // defined(LFM_IMPLEMENT)
