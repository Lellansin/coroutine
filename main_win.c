#include "coroutine_win.h"
#include "debug.h"
#include <stdio.h>

struct args {
	int n;
};

VOID
__stdcall
foo(LPVOID param) {
	struct schedule *S = coroutine_get_schedule(param);
	struct args* args = coroutine_get_args(param);
	int i;
	debug("fiber %d start\n", coroutine_running(S));
	for (i = 0; i < 5; ++i)
	{
		printf("coroutine %d : %d\n", coroutine_running(S) , args->n + i);
		coroutine_yield(S);
	}
	debug("fiber %d end\n", coroutine_running(S));
	coroutine_end(S);
}

static void
test(struct schedule *S) {
	struct args arg1 = { 0 };
	struct args arg2 = { 100 };

	int co1 = coroutine_new(S, foo, &arg1);
	int co2 = coroutine_new(S, foo, &arg2);
	printf("main start\n");
	while (coroutine_status(S,co1) && coroutine_status(S,co2)) {
		coroutine_resume(S,co1);
		coroutine_resume(S,co2);
	} 
	printf("main end\n");
}

int 
main() {
	struct schedule * S = coroutine_open();
	test(S);
	coroutine_close(S);
	
	return 0;
}

