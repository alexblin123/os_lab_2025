#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // Добавлено для использования true/false
#include <sys/time.h>
#include <getopt.h>
#include <pthread.h>

// Подключаем наши заголовочные файлы
#include "/workspaces/os_lab_2025/lab3/src/utils.h"
#include "sum.h"

// Глобальные переменные для аргументов командной строки
uint32_t threads_num = 0;
uint32_t array_size = 0;
uint32_t seed = 0;

void *ThreadSum(void *args) {
    struct SumArgs *sum_args = (struct SumArgs *)args;
    return (void *)(size_t)Sum(sum_args);
}

int main(int argc, char **argv) {
    // Обработка аргументов командной строки
    static struct option options[] = {
        {"threads_num", required_argument, 0, 0},
        {"array_size", required_argument, 0, 0},
        {"seed", required_argument, 0, 0},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    while (true) {
        int c = getopt_long(argc, argv, "", options, &option_index);
        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        threads_num = atoi(optarg);
                        if (threads_num <= 0) {
                            printf("threads_num must be a positive number\n");
                            return 1;
                        }
                        break;
                    case 1:
                        array_size = atoi(optarg);
                        if (array_size <= 0) {
                            printf("array_size must be a positive number\n");
                            return 1;
                        }
                        break;
                    case 2:
                        seed = atoi(optarg);
                        if (seed <= 0) {
                            printf("seed must be a positive number\n");
                            return 1;
                        }
                        break;
                }
                break;
            case '?':
                break;
            default:
                printf("getopt returned character code 0%o?\n", c);
        }
    }

    if (threads_num == 0 || array_size == 0 || seed == 0) {
        printf("Usage: %s --threads_num <num> --array_size <num> --seed <num>\n", argv[0]);
        return 1;
    }

    // Генерация массива (НЕ входит в замер времени)
    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);

    // Создание потоков
    pthread_t threads[threads_num];
    struct SumArgs args[threads_num];

    // Начало замера времени (только вычисление суммы)
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    // Распределение работы между потоками
    int chunk_size = array_size / threads_num;
    for (uint32_t i = 0; i < threads_num; i++) {
        args[i].array = array;
        args[i].begin = i * chunk_size;
        args[i].end = (i == threads_num - 1) ? array_size : (i + 1) * chunk_size;

        if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
            printf("Error: pthread_create failed!\n");
            free(array);
            return 1;
        }
    }

    // Сбор результатов
    int total_sum = 0;
    for (uint32_t i = 0; i < threads_num; i++) {
        int sum = 0;
        pthread_join(threads[i], (void **)&sum);
        total_sum += sum;
    }

    // Конец замера времени
    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    // Вычисление времени выполнения
    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);
    printf("Total: %d\n", total_sum);
    printf("Elapsed time: %fms\n", elapsed_time);
    
    return 0;
}