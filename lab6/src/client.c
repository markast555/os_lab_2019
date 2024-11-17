#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include "multmodulo.h"

struct Server {
    char ip[255];
    int port;
};

// uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
//     uint64_t result = 0;
//     a = a % mod;
//     while (b > 0) {
//         if (b % 2 == 1)
//             result = (result + a) % mod;
//         a = (a * 2) % mod;
//         b /= 2;
//     }
//     return result % mod;
// }

bool ConvertStringToUI64(const char *str, uint64_t *val) {
    char *end = NULL;
    unsigned long long i = strtoull(str, &end, 10);
    if (errno == ERANGE) {
        fprintf(stderr, "Out of uint64_t range: %s\n", str);
        return false;
    }
    if (errno != 0)
        return false;
    *val = i;
    return true;
}

struct ThreadData {
    struct Server server;
    uint64_t begin;
    uint64_t end;
    uint64_t mod;
    uint64_t result;
};

void *send_request(void *arg) {
    struct ThreadData *data = (struct ThreadData *)arg;

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(data->server.port);
    struct hostent *hostname = gethostbyname(data->server.ip);
    if (hostname == NULL) {
        fprintf(stderr, "gethostbyname failed with %s\n", data->server.ip);
        // pthread_exit(NULL);
        exit(1);
    }
    server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
        fprintf(stderr, "Socket creation failed!\n");
        // pthread_exit(NULL);
        exit(1);
    }

    if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
        fprintf(stderr, "Connection failed\n");
        // pthread_exit(NULL);
        exit(1);
    }

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &data->begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &data->end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &data->mod, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) < 0) {
        fprintf(stderr, "Send failed\n");
        // pthread_exit(NULL);
        exit(1);
    }

    char response[sizeof(uint64_t)];
    if (recv(sck, response, sizeof(response), 0) < 0) {
        fprintf(stderr, "Receive failed\n");
        // pthread_exit(NULL);
        exit(1);
    }

    memcpy(&data->result, response, sizeof(uint64_t));
    close(sck);
    // pthread_exit(NULL);
}

int main(int argc, char **argv) {
    uint64_t k = -1;
    uint64_t mod = -1;
    char servers[255] = {'\0'};

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {
            {"k", required_argument, 0, 0},
            {"mod", required_argument, 0, 0},
            {"servers", required_argument, 0, 0},
            {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
        case 0: {
            switch (option_index) {
            case 0:
                ConvertStringToUI64(optarg, &k);
                break;
            case 1:
                ConvertStringToUI64(optarg, &mod);
                break;
            case 2:
                memcpy(servers, optarg, strlen(optarg));
                break;
            default:
                printf("Index %d is out of options\n", option_index);
            }
        } break;

        case '?':
            printf("Arguments error\n");
            break;
        default:
            fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    if (k == -1 || mod == -1 || !strlen(servers)) {
        fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n", argv[0]);
        return 1;
    }

    // Чтение серверов из файла
    FILE *file = fopen(servers, "r");
    if (!file) {
        fprintf(stderr, "Could not open servers file\n");
        return 1;
    }

    unsigned int servers_num = 2;
    struct Server *to = malloc(sizeof(struct Server) * servers_num);
    int server_count = 0;

    while (fscanf(file, "%s %d", to[server_count].ip, &to[server_count].port) != EOF) {
        server_count++;
    }
    fclose(file);

    pthread_t *threads = malloc(sizeof(pthread_t) * server_count);
    struct ThreadData *thread_data = malloc(sizeof(struct ThreadData) * server_count);

    // Разделите задачу между серверами
    uint64_t chunk_size = k / server_count;
    for (int i = 0; i < server_count; i++) {
        thread_data[i].server = to[i];
        thread_data[i].begin = i * chunk_size + 1;
        thread_data[i].end = (i == server_count - 1) ? k : (i + 1) * chunk_size;
        thread_data[i].mod = mod;
        thread_data[i].result = 1; // Инициализация результата

        pthread_create(&threads[i], NULL, send_request, &thread_data[i]);
    }

    uint64_t total = 1;
    for (int i = 0; i < server_count; i++) {
        pthread_join(threads[i], NULL);
        total = MultModulo(total, thread_data[i].result, mod);
    }

    printf("Final answer: %lu\n", (unsigned long)total);

    free(threads);
    free(thread_data);
    free(to);

    return 0;
}
