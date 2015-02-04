all : main

#
# default to compile windows vershion now
#
main : main_win.c coroutine_win.c debug.c
	gcc -g -Wall -o $@ $^
	@echo -------------------
	main

unix: main.c coroutine.c
	gcc -g -Wall -o $@ $^

win: main_win.c coroutine_win.c debug.c
	gcc -g -Wall -o $@ $^
	@echo -------------------
	win

clean :
	rm main

d: main_win.c coroutine_win.c debug.c
	gcc -g -Wall -o $@ $^ -D _debug
	@echo -------------------
	@d