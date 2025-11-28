#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>


typedef struct {
    int thread_id;      
    int k;              
    int pnum;           
    int mod;            
    long long *result;  
    pthread_mutex_t *mutex; 
} thread_data_t;

void* calculate_partial_factorial(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    
    // Определяем диапазон для текущего потока
    int numbers_per_thread = data->k / data->pnum;
    int start = data->thread_id * numbers_per_thread + 1;
    int end = (data->thread_id == data->pnum - 1) ? data->k : (data->thread_id + 1) * numbers_per_thread;
    
    printf("Поток %d: вычисляет числа от %d до %d\n", data->thread_id, start, end);
    
    // Вычисляем частичный факториал для своего диапазона
    long long partial_result = 1;
    for (int i = start; i <= end; i++) {
        partial_result = (partial_result * i) % data->mod;
    }
    
    printf("Поток %d: частичный результат = %lld\n", data->thread_id, partial_result);
    
    // Захватываем мьютекс для безопасного обновления общего результата
    pthread_mutex_lock(data->mutex);
    *(data->result) = (*(data->result) * partial_result) % data->mod;
    pthread_mutex_unlock(data->mutex);
    
    printf("Поток %d: завершил работу\n", data->thread_id);
    
    return NULL;
}

// Функция для парсинга аргументов командной строки
int parse_arguments(int argc, char* argv[], int* k, int* pnum, int* mod) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0 && i + 1 < argc) {
            *k = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--pnum") == 0 && i + 1 < argc) {
            *pnum = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--mod") == 0 && i + 1 < argc) {
            *mod = atoi(argv[++i]);
        }
    }
    
    if (*k <= 0 || *pnum <= 0 || *mod <= 0) {
        printf("Ошибка: все параметры должны быть положительными числами\n");
        return -1;
    }
    
    return 0;
}

int main(int argc, char* argv[]) {
    int k = 0, pnum = 0, mod = 0;
    
    if (parse_arguments(argc, argv, &k, &pnum, &mod) != 0) {
        printf("Использование: %s -k <число> --pnum <потоки> --mod <модуль>\n", argv[0]);
        printf("Пример: %s -k 10 --pnum 4 --mod 1000000\n", argv[0]);
        return 1;
    }
    
    printf("Вычисление %d! по модулю %d с использованием %d потоков\n", k, mod, pnum);
    
    long long result = 1;
    
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    
    pthread_t threads[pnum];
    thread_data_t thread_data[pnum];
    
    for (int i = 0; i < pnum; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].k = k;
        thread_data[i].pnum = pnum;
        thread_data[i].mod = mod;
        thread_data[i].result = &result;
        thread_data[i].mutex = &mutex;
        
        if (pthread_create(&threads[i], NULL, calculate_partial_factorial, &thread_data[i]) != 0) {
            perror("Ошибка при создании потока");
            return 1;
        }
    }
    
    for (int i = 0; i < pnum; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Ошибка при ожидании потока");
            return 1;
        }
    }
    
    pthread_mutex_destroy(&mutex);
    
    printf("\nРезультат: %d! mod %d = %lld\n", k, mod, result);
    
    return 0;
}