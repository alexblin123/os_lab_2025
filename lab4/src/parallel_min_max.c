#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "/workspaces/os_lab_2025/lab3/src/find_min_max.h"
#include "/workspaces/os_lab_2025/lab3/src/utils.h"

volatile sig_atomic_t timeout_expired = 0;

void handle_sigalrm(int sig) {
    timeout_expired = 1;
}

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    int timeout = 0; // Таймаут по умолчанию
    bool with_files = false;

    static struct option options[] = {
        {"seed", required_argument, 0, 0},
        {"array_size", required_argument, 0, 0},
        {"pnum", required_argument, 0, 0},
        {"by_files", no_argument, 0, 'f'},
        {"timeout", required_argument, 0, 't'},
        {0, 0, 0, 0}
    };

    while (true) {
        int current_optind = optind ? optind : 1;
        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0: seed = atoi(optarg); break;
                    case 1: array_size = atoi(optarg); break;
                    case 2: pnum = atoi(optarg); break;
                    case 3: with_files = true; break;
                }
                break;
            case 'f': with_files = true; break;
            case 't': timeout = atoi(optarg); break;
        }
    }

    // Проверка обязательных аргументов
    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed <num> --array_size <num> --pnum <num> [--timeout <sec>]\n", argv[0]);
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);

    int pipes[pnum][2];
    if (!with_files) {
        for (int i = 0; i < pnum; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                return 1;
            }
        }
    }

    pid_t child_pids[pnum];
    int active_child_processes = 0;

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    // Установка обработчика сигнала таймаута
    signal(SIGALRM, handle_sigalrm);

    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();
        if (child_pid < 0) {
            perror("fork");
            return 1;
        }

        if (child_pid == 0) {
            // Дочерний процесс
            if (timeout > 0) {
                alarm(0); // Отключаем унаследованный будильник
            }
            int start = i * (array_size / pnum);
            int end = (i == pnum - 1) ? array_size : (i + 1) * (array_size / pnum);
            struct MinMax local_min_max = GetMinMax(array, start, end);

            if (with_files) {
                char filename[20];
                sprintf(filename, "minmax_%d.txt", i);
                FILE *file = fopen(filename, "w");
                fprintf(file, "%d %d", local_min_max.min, local_min_max.max);
                fclose(file);
            } else {
                close(pipes[i][0]);
                write(pipes[i][1], &local_min_max, sizeof(struct MinMax));
                close(pipes[i][1]);
            }
            free(array);
            exit(0);
        } else {
            // Родительский процесс
            child_pids[i] = child_pid;
            active_child_processes++;
        }
    }

    // Установка таймаута
    if (timeout > 0) {
        alarm(timeout);
    }

    // Ожидание завершения дочерних процессов
    while (active_child_processes > 0) {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);

        if (pid > 0) {
            active_child_processes--;
        } else if (timeout_expired) {
            printf("Timeout expired. Killing child processes...\n");
            for (int i = 0; i < pnum; i++) {
                kill(child_pids[i], SIGKILL);
            }
            break;
        }
        usleep(10000); // 10 ms пауза для уменьшения нагрузки на CPU
    }

    // Сбор результатов
    struct MinMax min_max = {INT_MAX, INT_MIN};
    for (int i = 0; i < pnum; i++) {
        if (with_files) {
            char filename[20];
            sprintf(filename, "minmax_%d.txt", i);
            FILE *file = fopen(filename, "r");
            if (file) {
                int min, max;
                fscanf(file, "%d %d", &min, &max);
                fclose(file);
                remove(filename);
                if (min < min_max.min) min_max.min = min;
                if (max > min_max.max) min_max.max = max;
            }
        } else {
            close(pipes[i][1]);
            struct MinMax local_min_max;
            if (read(pipes[i][0], &local_min_max, sizeof(struct MinMax)) > 0) {
                if (local_min_max.min < min_max.min) min_max.min = local_min_max.min;
                if (local_min_max.max > min_max.max) min_max.max = local_min_max.max;
            }
            close(pipes[i][0]);
        }
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);
    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);
    return 0;
}