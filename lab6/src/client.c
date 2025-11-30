#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include "utils.h"
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <inttypes.h>

struct Server {
  char ip[255];
  int port;
};


int main(int argc, char **argv) {
  // Использование 0 для инициализации вместо -1, чтобы избежать предупреждений сравнения
  uint64_t k = 0;
  uint64_t mod = 0;
  char servers_file[255] = {'\0'}; 

  while (true) {
    int current_optind = optind ? optind : 1; // Убрать warning: unused variable

    static struct option options[] = {{"k", required_argument, 0, 0},
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
        memcpy(servers_file, optarg, strlen(optarg));
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

  // Проверка аргументов: теперь проверяем k > 0, mod > 0
  if (k == 0 || mod == 0 || !strlen(servers_file)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  // === ЧТЕНИЕ ФАЙЛА С СЕРВЕРАМИ ===
  unsigned int servers_num = 0;
  FILE *fp = fopen(servers_file, "r");
  if (!fp) {
      perror("Failed to open servers file");
      return 1;
  }
  
  // Чтение с запасом, предполагаем максимум 100 серверов
  struct Server *to = malloc(sizeof(struct Server) * 100);
  
  char line[256];
  while (fgets(line, sizeof(line), fp)) {
      char *ip = strtok(line, ":");
      char *port_str = strtok(NULL, ":\n");
      if (ip && port_str) {
          strcpy(to[servers_num].ip, ip);
          to[servers_num].port = atoi(port_str);
          servers_num++;
      }
  }
  fclose(fp);
  
  if (servers_num == 0) {
      fprintf(stderr, "No servers found in file\n");
      free(to);
      return 1;
  }

  int *sockets = malloc(sizeof(int) * servers_num);
  uint64_t range = k / servers_num; 
  
  // === 1. ОТПРАВКА ЗАДАЧ (ПАРАЛЛЕЛЬНОСТЬ) ===
  for (unsigned int i = 0; i < servers_num; i++) { // Заменили int i на unsigned int i
    struct hostent *hostname = gethostbyname(to[i].ip);
    if (hostname == NULL) {
      fprintf(stderr, "gethostbyname failed with %s\n", to[i].ip);
      exit(1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(to[i].port);
    
    // КРИТИЧЕСКОЕ ИЗМЕНЕНИЕ: Использование h_addr_list[0] вместо h_addr
    server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr_list[0]); 

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
      fprintf(stderr, "Socket creation failed!\n");
      exit(1);
    }
    
    sockets[i] = sck;

    if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
      fprintf(stderr, "Connection failed\n");
      exit(1);
    }

    uint64_t begin = 1 + i * range;
    uint64_t end = (i == servers_num - 1) ? k : (begin + range - 1);

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) < 0) {
      fprintf(stderr, "Send failed\n");
      exit(1);
    }
    // Используем макрос PRIX64 для корректного вывода uint64_t

    printf("Sent task to %s:%d : %" PRIu64 "-%" PRIu64 "\n", to[i].ip, to[i].port, begin, end);
  }

  // === 2. СБОР РЕЗУЛЬТАТОВ ===
  uint64_t total_answer = 1;
  
  for (unsigned int i = 0; i < servers_num; i++) { // Заменили int i на unsigned int i
      char response[sizeof(uint64_t)];
      if (recv(sockets[i], response, sizeof(response), 0) < 0) {
          fprintf(stderr, "Recieve failed\n");
          exit(1);
      }
      
      uint64_t answer = 0;
      memcpy(&answer, response, sizeof(uint64_t));
      
      total_answer = MultModulo(total_answer, answer, mod);
      
      close(sockets[i]);
  }

  // Используем макрос PRIX64 для корректного вывода uint64_t
  printf(" %" PRIu64 "\n", total_answer);

  free(to);
  free(sockets);

  return 0;
}