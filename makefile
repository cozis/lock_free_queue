
all:
	gcc lock_free_queue_test.c lock_free_queue.c tinycsem.c tinycthread.c -o test_queue -Wall -Wextra