#include "coroutine_win.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
// #include <stdint.h>

#define STACK_SIZE (1024*1024)
#define DEFAULT_COROUTINE 16

struct coroutine;

struct schedule {
    char stack[STACK_SIZE];
    // ucontext_t main;
    int nco;
    int cap;
    int running;
    struct coroutine **co;
    void *master;
};

struct coroutine {
	// coroutine_func func;
	LPFIBER_START_ROUTINE func;
	void *ud;
	// ucontext_t ctx;
	struct schedule * sch;
	ptrdiff_t cap;
	ptrdiff_t size;
	int status;
	char *stack;
	void *fiber;
	void *master;
};

struct coroutine_param {
    struct schedule *S;
    void * args;
};

struct coroutine * 
_co_new(struct schedule *S , LPFIBER_START_ROUTINE func, void *ud) {
	struct coroutine * co = malloc(sizeof(*co));
	co->func = func;
	co->ud = ud;
	co->sch = S;
	co->cap = 0;
	co->size = 0;
	co->status = COROUTINE_READY;
	co->stack = NULL;
	co->fiber = CreateFiber(0, func, ud);
	co->master = S->master;
	return co;
}

void
_co_delete(struct coroutine *co) {
	// free(co->stack);
	DeleteFiber(co->fiber);
	free(co);
}

struct schedule * 
coroutine_open(void) {
	int nothing;
	struct schedule *S = malloc(sizeof(*S));
	S->nco = 0;
	S->cap = DEFAULT_COROUTINE;
	S->running = -1;
	S->co = malloc(sizeof(struct coroutine *) * S->cap);
	memset(S->co, 0, sizeof(struct coroutine *) * S->cap);
	
	S->master = ConvertThreadToFiber(&nothing);
	return S;
}

void 
coroutine_close(struct schedule *S) {
	int i;
	for (i=0;i<S->cap;i++) {
		struct coroutine * co = S->co[i];
		if (co) {
			_co_delete(co);
		}
	}
	free(S->co);
	S->co = NULL;
	free(S);
}

int 
coroutine_new(struct schedule *S, LPFIBER_START_ROUTINE func, void *ud) {
	struct coroutine_param *param = malloc(sizeof(struct coroutine_param));
	struct coroutine *co = _co_new(S, func , param);
	param->S = S;
	param->args = ud;
	if (S->nco >= S->cap) {
		int id = S->cap;
		S->co = realloc(S->co, S->cap * 2 * sizeof(struct coroutine *));
		memset(S->co + S->cap , 0 , sizeof(struct coroutine *) * S->cap);
		S->co[S->cap] = co;
		S->cap *= 2;
		++S->nco;
		return id;
	} else {
		int i;
		for (i=0;i<S->cap;i++) {
			int id = (i+S->nco) % S->cap;
			if (S->co[id] == NULL) {
				S->co[id] = co;
				++S->nco;
				return id;
			}
		}
	}
	assert(0);
	return -1;
}

struct schedule * coroutine_get_schedule(void * param) {
	return ((struct coroutine_param *)param)->S;
}

void * coroutine_get_args(void * param) {
	return ((struct coroutine_param *)param)->args;
}

void 
coroutine_resume(struct schedule * S, int id) {
	assert(S->running == -1);
	assert(id >=0 && id < S->cap);
	struct coroutine *C = S->co[id];
	if (C == NULL)
		return;
	int status = C->status;
	switch(status) {
	case COROUTINE_READY:
		S->running = id;
		C->status = COROUTINE_RUNNING;
		SwitchToFiber(C->fiber);
		C->status = COROUTINE_READY;
		S->running = -1;
		break;
	case COROUTINE_SUSPEND:
		S->running = id;
		C->status = COROUTINE_RUNNING;
		SwitchToFiber(C->fiber);
		SwitchToFiber(S->master);
		break;
	default:
		assert(0);
	}
}

void
coroutine_end(struct schedule * S) {
	int id = S->running;
	assert(id >=0 && id < S->cap);
	struct coroutine *C = S->co[S->running];
	if (C == NULL){
		;
	} else {
		C->status = COROUTINE_DEAD;
		// DeleteFiber(C->fiber);
	}
	S->co[id] = NULL;
	SwitchToFiber(S->master);
}

void
coroutine_yield(struct schedule * S) {
	int id = S->running;
	assert(id >= 0);
	struct coroutine * C = S->co[id];
	C->status = COROUTINE_SUSPEND;
	S->running = -1;
	SwitchToFiber(S->master);
}

int 
coroutine_status(struct schedule * S, int id) {
	assert(id>=0 && id < S->cap);
	if (S->co[id] == NULL) {
		return COROUTINE_DEAD;
	}
	return S->co[id]->status;
}

int 
coroutine_running(struct schedule * S) {
	return S->running;
}

