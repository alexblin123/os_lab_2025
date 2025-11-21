#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void demonstrate_zombie() {
    printf("=== Демонстрация создания зомби-процесса ===\n");
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Дочерний процесс
        printf("Дочерний процесс: PID = %d, PPID = %d\n", getpid(), getppid());
        printf("Дочерний процесс завершается...\n");
        exit(0);  // Завершаем дочерний процесс
    } else if (pid > 0) {
        // Родительский процесс
        printf("Родительский процесс: PID = %d, создал дочерний с PID = %d\n", getpid(), pid);
        printf("Родительский процесс НЕ вызывает wait() и спит 30 секунд...\n");
        printf("В это время дочерний процесс станет зомби!\n");
        
        // Не вызываем wait() - создаем зомби
        sleep(30);
        
        printf("Родительский процесс завершается.\n");
    } else {
        perror("fork failed");
        exit(1);
    }
}

void prevent_zombie_with_wait() {
    printf("\n=== Демонстрация предотвращения зомби с помощью wait() ===\n");
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Дочерний процесс
        printf("Дочерний процесс: PID = %d\n", getpid());
        printf("Дочерний процесс завершается...\n");
        exit(0);
    } else if (pid > 0) {
        // Родительский процесс
        printf("Родительский процесс создал дочерний с PID = %d\n", pid);
        
        int status;
        pid_t finished_pid = wait(&status);  // Ждем завершения дочернего процесса
        
        printf("Родительский процесс: дочерний процесс %d завершился со статусом %d\n", 
               finished_pid, WEXITSTATUS(status));
        printf("Зомби-процесс не создан!\n");
    } else {
        perror("fork failed");
        exit(1);
    }
}

void prevent_zombie_with_sigchld() {
    printf("\n=== Демонстрация предотвращения зомби с помощью SIGCHLD ===\n");
    
    // Игнорируем сигнал SIGCHLD - система автоматически убирает завершенные процессы
    signal(SIGCHLD, SIG_IGN);
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Дочерний процесс
        printf("Дочерний процесс: PID = %d\n", getpid());
        printf("Дочерний процесс завершается...\n");
        exit(0);
    } else if (pid > 0) {
        // Родительский процесс
        printf("Родительский процесс создал дочерний с PID = %d\n", pid);
        printf("SIGCHLD игнорируется - система автоматически уберет дочерний процесс\n");
        
        sleep(2);  // Даем время дочернему процессу завершиться
        
        printf("Родительский процесс продолжает работу...\n");
    } else {
        perror("fork failed");
        exit(1);
    }
}

void multiple_zombies() {
    printf("\n=== Демонстрация создания нескольких зомби-процессов ===\n");
    
    int num_children = 5;
    printf("Создаем %d дочерних процессов без wait()...\n", num_children);
    
    for (int i = 0; i < num_children; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            // Дочерний процесс
            printf("Дочерний процесс %d: PID = %d\n", i, getpid());
            exit(0);
        } else if (pid > 0) {
            printf("Создан дочерний процесс %d с PID = %d\n", i, pid);
        } else {
            perror("fork failed");
            exit(1);
        }
    }
    
    printf("Все дочерние процессы завершены, но родитель НЕ вызывает wait()\n");
    printf("Сейчас %d процессов стали зомби!\n", num_children);
    printf("Родительский процесс спит 20 секунд...\n");
    sleep(20);
    
    // Теперь уберем зомби
    printf("Теперь убираем зомби с помощью waitpid()...\n");
    for (int i = 0; i < num_children; i++) {
        int status;
        pid_t finished_pid = waitpid(-1, &status, WNOHANG);
        if (finished_pid > 0) {
            printf("Убран зомби-процесс: PID = %d\n", finished_pid);
        }
    }
}

int main() {
    printf("Демонстрация зомби-процессов\n");
    printf("============================\n\n");
    
    int choice;
    printf("Выберите демонстрацию:\n");
    printf("1 - Создание одного зомби-процесса\n");
    printf("2 - Предотвращение зомби с помощью wait()\n");
    printf("3 - Предотвращение зомби с помощью SIGCHLD\n");
    printf("4 - Создание нескольких зомби-процессов\n");
    printf("Ваш выбор: ");
    
    scanf("%d", &choice);
    
    switch(choice) {
        case 1:
            demonstrate_zombie();
            break;
        case 2:
            prevent_zombie_with_wait();
            break;
        case 3:
            prevent_zombie_with_sigchld();
            break;
        case 4:
            multiple_zombies();
            break;
        default:
            printf("Неверный выбор!\n");
            return 1;
    }
    
    return 0;
}