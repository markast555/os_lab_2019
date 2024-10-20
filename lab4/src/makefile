CC=gcc
CFLAGS=-I.

new :  parallel_min_max_2 zombie process_memory

parallel_min_max_2 : parallel_min_max_2.c utils_2.o find_min_max_2.o utils_2.h find_min_max_2.h
	$(CC) -o parallel_min_max_2 utils_2.o find_min_max_2.o parallel_min_max_2.c $(CFLAGS)

zombie : zombie.c
	$(CC) -o zombie -c zombie.c $(CFLAGS)

process_memory : process_memory.c
	$(CC) -o process_memory -c process_memory.c $(CFLAGS)

utils_2.o : utils_2.h
	$(CC) -o utils_2.o -c utils_2.c $(CFLAGS)

find_min_max_2.o : utils_2.h find_min_max_2.h
	$(CC) -o find_min_max_2.o -c find_min_max_2.c $(CFLAGS)

clean :
	rm utils_2.o find_min_max_2.o parallel_min_max_2 zombie process_memory
