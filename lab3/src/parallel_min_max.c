#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
    
    int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("seed is a positive number\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("array_size is a positive number\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 1) {
              printf("pnum is more than one\n");
              return 1;
            }
            break;
          case 3:
            with_files = true;
            break;

          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);
    int active_child_processes = 0;
    int pipe_fd[2];

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    if (!with_files) {
        pipe(pipe_fd); // создаем pipe
    }

    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();
        if (child_pid >= 0) {
            active_child_processes += 1;
            if (child_pid == 0) {
                // child process
                int start = i * (array_size / pnum);
                int end = (i + 1) * (array_size / pnum);
                if (i == pnum - 1) end = array_size; // последний процесс берет остаток

                int local_min = INT_MAX;
                int local_max = INT_MIN;

                for (int j = start; j < end; j++) {
                    if (array[j] < local_min) local_min = array[j];
                    if (array[j] > local_max) local_max = array[j];
                }

                if (with_files) {
                    // запись результатов в файл
                    char filename[20];
                    sprintf(filename, "result_%d.txt", i);
                    FILE *file = fopen(filename, "w");
                    fprintf(file, "%d %d\n", local_min, local_max);
                    fclose(file);
                } else {
                    // отправка результатов через pipe
                    write(pipe_fd[1], &local_min, sizeof(local_min));
                    write(pipe_fd[1], &local_max, sizeof(local_max));
                }

                exit(0);
            }
        } else {
            printf("Fork failed!\n");
            return 1;
        }
    }

    while (active_child_processes > 0) {
        wait(NULL); // ожидание завершения дочерних процессов
        active_child_processes -= 1;
    }

    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        if (with_files) {
            // чтение результатов из файлов
            char filename[20];
            sprintf(filename, "result_%d.txt", i);
            FILE *file = fopen(filename, "r");
            int local_min, local_max;
            fscanf(file, "%d %d", &local_min, &local_max);
            fclose(file);

            if (local_min < min_max.min) min_max.min = local_min;
            if (local_max > min_max.max) min_max.max = local_max;
        } else {
            // чтение результатов из pipe
            int local_min, local_max;
            read(pipe_fd[0], &local_min, sizeof(local_min));
            read(pipe_fd[0], &local_max, sizeof(local_max));

            if (local_min < min_max.min) min_max.min = local_min;
            if (local_max > min_max.max) min_max.max = local_max;
        }
    }

    if (!with_files) {
        close(pipe_fd[0]); // закрываем pipe
        close(pipe_fd[1]); // закрываем pipe
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);

    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);
    fflush(NULL);
    return 0;
}
