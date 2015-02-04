#ifndef C_COROUTINE_H
#define C_COROUTINE_H

#include <windows.h>

#define COROUTINE_DEAD 0
#define COROUTINE_READY 1
#define COROUTINE_RUNNING 2
#define COROUTINE_SUSPEND 3

struct coroutine_param;
struct schedule;

// typedef void (*coroutine_func)(struct schedule *, void *ud);

struct schedule * coroutine_open(void);
void coroutine_close(struct schedule *);

// int coroutine_new(struct schedule *, coroutine_func, void *ud);
int coroutine_new(struct schedule *, LPFIBER_START_ROUTINE, void *ud);
void coroutine_resume(struct schedule *, int id);
struct schedule * coroutine_get_schedule(void * param);
void * coroutine_get_args(void * param);
void coroutine_end(struct schedule *);
int coroutine_status(struct schedule *, int id);
int coroutine_running(struct schedule *);
void coroutine_yield(struct schedule *);


#endif
