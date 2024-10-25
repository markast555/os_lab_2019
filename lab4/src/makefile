CC = gcc
CFLAGS = -I.

new: parallel_min_max_2 zombie process_memory psum

parallel_min_max_2: parallel_min_max_2.c utils_2.o find_min_max_2.o utils_2.h find_min_max_2.h
	$(CC) -o parallel_min_max_2 utils_2.o find_min_max_2.o parallel_min_max_2.c $(CFLAGS)

zombie: zombie.c
	$(CC) -o zombie zombie.c $(CFLAGS)

process_memory: process_memory.c
	$(CC) -o process_memory process_memory.c $(CFLAGS)

psum: parallel_sum.o sum.o utils_2.o sum.h utils_2.h
	$(CC) -o psum parallel_sum.o sum.o utils_2.o -lpthread $(CFLAGS)


utils_2.o: utils_2.c utils_2.h
	$(CC) -c utils_2.c $(CFLAGS)

find_min_max_2.o: find_min_max_2.c find_min_max_2.h utils_2.h
	$(CC) -c find_min_max_2.c $(CFLAGS)

sum.o: sum.c sum.h
	$(CC) -c sum.c $(CFLAGS)

parallel_sum.o: parallel_sum.c sum.h utils_2.h
	$(CC) -c parallel_sum.c $(CFLAGS)

clean:
	rm -f utils_2.o find_min_max_2.o parallel_min_max_2 zombie process_memory parallel_sum.o sum.o psum
