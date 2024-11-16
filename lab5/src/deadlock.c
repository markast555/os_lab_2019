#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutexA = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexB = PTHREAD_MUTEX_INITIALIZER;

void* thread_func_1(void* arg) {
    printf("Поток 1: Пытаюсь заблокировать Мьютекс A...\n");
    pthread_mutex_lock(&mutexA);
    printf("Поток 1: Заблокирован Мьютекс A.\n");

    sleep(1); // Задержка, чтобы гарантировать, что второй поток сможет захватить Мьютекс B

    printf("Поток 1: Пытаюсь заблокировать Мьютекс B...\n");
    pthread_mutex_lock(&mutexB);
    printf("Поток 1: Заблокирован Мьютекс B.\n");

    pthread_mutex_unlock(&mutexB);
    pthread_mutex_unlock(&mutexA);
    return NULL;
}

void* thread_func_2(void* arg) {
    printf("Поток 2: Пытаюсь заблокировать Мьютекс B...\n");
    pthread_mutex_lock(&mutexB);
    printf("Поток 2: Заблокирован Мьютекс B.\n");

    sleep(1); // Задержка, чтобы гарантировать, что первый поток сможет захватить Мьютекс A

    printf("Поток 2: Пытаюсь заблокировать Мьютекс A...\n");
    pthread_mutex_lock(&mutexA);
    printf("Поток 2: Заблокирован Мьютекс A.\n");

    pthread_mutex_unlock(&mutexA);
    pthread_mutex_unlock(&mutexB);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, thread_func_1, NULL);
    pthread_create(&thread2, NULL, thread_func_2, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&mutexA);
    pthread_mutex_destroy(&mutexB);

    return 0;
}
