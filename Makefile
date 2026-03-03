CC=cc
CSTD=-std=c11
COMMON=$(CSTD) -Wall -Werror -Wextra
UBSAN_FLAGS=$(COMMON) -fsanitize=undefined -fno-omit-frame-pointer -O0 -g
TSAN_FLAGS=$(COMMON) -fsanitize=thread -O0 -g
VALGRIND_FLAGS=$(COMMON) -O0 -g
VALGRIND_RUN_FLAGS=--tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --fair-sched=yes
DEBUG_FLAGS=$(COMMON) -O0 -g
RELEASE_FLAGS=$(COMMON) -O3

release: ring_buffer.c ring_buffer.h test.c
	$(CC) $(RELEASE_FLAGS) ring_buffer.c test.c -o test

debug: ring_buffer.c ring_buffer.h test.c
	$(CC) $(DEBUG_FLAGS) -DDEBUG ring_buffer.c test.c -o test

ubsan-build: ring_buffer.c ring_buffer.h test.c
	$(CC) $(UBSAN_FLAGS) ring_buffer.c test.c -o test_ubsan

ubsan: ubsan-build
	./test_ubsan

tsan-build: ring_buffer.c ring_buffer.h test.c
	$(CC) $(TSAN_FLAGS) ring_buffer.c test.c -o test_tsan

tsan: tsan-build
	./test_tsan

valgrind-build: ring_buffer.c ring_buffer.h test.c
	$(CC) $(VALGRIND_FLAGS) ring_buffer.c test.c -o test_valgrind

valgrind: valgrind-build
	valgrind $(VALGRIND_RUN_FLAGS) ./test_valgrind

clean:
	rm -f test test_ubsan test_tsan test_valgrind

.PHONY: release debug ubsan-build ubsan tsan-build tsan valgrind-build valgrind clean
