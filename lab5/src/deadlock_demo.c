#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* thread1_function(void* arg) {
    printf("Поток 1: пытаюсь захватить мьютекс 1...\n");
    pthread_mutex_lock(&mutex1);
    printf("Поток 1: захватил мьютекс 1\n");
    
    sleep(1);
    
    printf("Поток 1: пытаюсь захватить мьютекс 2...\n");
    pthread_mutex_lock(&mutex2);
    printf("Поток 1: захватил мьютекс 2\n");
    
    printf("Поток 1: выполняю критическую секцию\n");
    
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    
    printf("Поток 1: завершил работу\n");
    
    return NULL;
}

void* thread2_function(void* arg) {
    printf("Поток 2: пытаюсь захватить мьютекс 2...\n");
    pthread_mutex_lock(&mutex2);
    printf("Поток 2: захватил мьютекс 2\n");
    
    sleep(1);
    
    printf("Поток 2: пытаюсь захватить мьютекс 1...\n");
    pthread_mutex_lock(&mutex1);
    printf("Поток 2: захватил мьютекс 1\n");
    
    printf("Поток 2: выполняю критическую секцию\n");
    
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    
    printf("Поток 2: завершил работу\n");
    
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    
    if (pthread_create(&thread1, NULL, thread1_function, NULL) != 0) {
        perror("Ошибка при создании потока 1");
        return 1;
    }
    
    if (pthread_create(&thread2, NULL, thread2_function, NULL) != 0) {
        perror("Ошибка при создании потока 2");
        return 1;
    }
    
    // Ожидаем завершения потоков (но они никогда не завершатся из-за deadlock)
    printf("Основной поток: ожидаю завершения потоков...\n");
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    // Эта строка никогда не будет выполнена из-за deadlock
    printf("Все потоки завершились\n");
    
    return 0;
}