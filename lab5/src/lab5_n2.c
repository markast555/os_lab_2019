#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <string.h>

typedef struct {
    int start;
    int end;
} ThreadData;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int result = 1;
int mod = -1;

void* factorial_partial(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    long long partial_result = 1;

    for (int i = data->start; i <= data->end; i++) {
        partial_result = (partial_result * i) % mod;
    }

    pthread_mutex_lock(&mutex);
    result = (result * partial_result) % mod;
    pthread_mutex_unlock(&mutex);

}

void factorial(int k, int pnum, int mod) {
    pthread_t threads[pnum];
    ThreadData thread_data[pnum];
    int range = k / pnum;
    
    for (int i = 0; i < pnum; i++) {
        thread_data[i].start = i * range + 1;
        thread_data[i].end = (i == pnum - 1) ? k : (i + 1) * range;

        if (pthread_create(&threads[i], NULL, factorial_partial, &thread_data[i]) != 0) {
            perror("pthread_create");
            exit(1);
        }
    }

    for (int i = 0; i < pnum; i++) {
        pthread_join(threads[i], NULL);
    }

    // return result;
}

int main(int argc, char* argv[]) {
    int k = -1, pnum = -1;

    while (1) {
        static struct option options[] = {
            {"k", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"mod", required_argument, 0, 0},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        k = atoi(optarg);
                        break;
                    case 1:
                        pnum = atoi(optarg);
                        break;
                    case 2:
                        mod = atoi(optarg);
                        break;
                    default:
                        fprintf(stderr, "Unknown option\n");
                        return 1;
                }
                break;

            case '?':
                // Неизвестный параметр
                break;

            default:
                fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    if (k < 0 || pnum <= 0 || mod <= 0) {
        fprintf(stderr, "Invalid input values. Usage: %s --k=<number> --pnum=<threads> --mod=<modulus>\n", argv[0]);
        return 1;
    }

    factorial(k, pnum, mod);
    printf("Факториал %d по модулю %d равен: %d\n", k, mod, result);

    // pthread_mutex_destroy(&mutex);
    return 0;
}
