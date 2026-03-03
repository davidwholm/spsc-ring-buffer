CC=cc
CFLAGS=-Wall -Werror -fsanitize=undefined,thread -O0 -g
V=valgrind
VFLAGS=--tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --fair-sched=yes

test: ring_buffer test.c
	$(CC) $(CFLAGS) ring_buffer.o test.c -o test

valgrind: test
	$(V) $(VFLAGS) test

ring_buffer: ring_buffer.c ring_buffer.h
	$(CC) $(CFLAGS) -c ring_buffer.c -o ring_buffer.o

clean:
	rm test *.o
