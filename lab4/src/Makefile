CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread
LDFLAGS = -pthread

LAB3_SRC = /workspaces/os_lab_2025/lab3/src
LAB3_HEADERS = /workspaces/os_lab_2025/lab3/src

TARGETS = parallel_min_max process_memory parallel_sum

all: $(TARGETS)

parallel_min_max: parallel_min_max.c $(LAB3_SRC)/find_min_max.c $(LAB3_SRC)/utils.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(LAB3_HEADERS)

process_memory: process_memory.c
	$(CC) $(CFLAGS) -o $@ $<

parallel_sum: parallel_sum.c sum.c $(LAB3_SRC)/utils.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(LAB3_HEADERS) -I.

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean