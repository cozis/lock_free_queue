
all:
	gcc lock_free_queue_test.c lock_free_queue.c tinycthread.c -o test_queue -ggdb -Wall -Wextra #-fsanitize=thread