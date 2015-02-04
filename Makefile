all : main

# main : main_win.c coroutine.c
main : main_win.c coroutine_win.c debug.c
	gcc -g -Wall -o $@ $^
	@echo -------------------
	main

clean :
	rm main

d: main_win.c coroutine_win.c debug.c
	gcc -g -Wall -o $@ $^ -D _debug
	@echo -------------------
	@d