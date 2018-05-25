execute: clean initializer process_generator spy finalizer

clean:
	rm -f initializer process_generator spy finalizer

initializer: functions.c initializer.c
	gcc -o initializer functions.c initializer.c

process_generator: functions.c process_generator.c
	gcc -o process_generator functions.c process_generator.c -lpthread

spy: functions.c spy.c
	gcc -o spy functions.c spy.c -lpthread

finalizer: functions.c finalizer.c
	gcc -o finalizer functions.c finalizer.c -lpthread