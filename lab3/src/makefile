CC=gcc
CFLAGS=-I.

all: sequential_min_max parallel_min_max launch_sequential

sequential_min_max: sequential_min_max.o utils.o find_min_max.o
	$(CC) -o sequential_min_max sequential_min_max.o utils.o find_min_max.o

parallel_min_max: parallel_min_max.o utils.o find_min_max.o
	$(CC) -o parallel_min_max parallel_min_max.o utils.o find_min_max.o

launch_sequential: launch_sequential.o
	$(CC) -o launch_sequential launch_sequential.o

sequential_min_max.o: sequential_min_max.c utils.h find_min_max.h
	$(CC) $(CFLAGS) -c sequential_min_max.c -o sequential_min_max.o

parallel_min_max.o: parallel_min_max.c utils.h find_min_max.h
	$(CC) $(CFLAGS) -c parallel_min_max.c -o parallel_min_max.o

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c -o utils.o

find_min_max.o: find_min_max.c find_min_max.h utils.h
	$(CC) $(CFLAGS) -c find_min_max.c -o find_min_max.o

launch_sequential.o: launch_sequential.c
	$(CC) $(CFLAGS) -c launch_sequential.c -o launch_sequential.o

clean:
	rm -f *.o sequential_min_max parallel_min_max launch_sequential