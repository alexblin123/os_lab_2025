# Переменные
CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = tcpserver tcpclient udpserver udpclient

# Цель по умолчанию: скомпилировать все
all: $(TARGETS)

# Правило для TCP-сервера
tcpserver: tcpserver.c
	$(CC) $(CFLAGS) tcpserver.c -o tcpserver

# Правило для TCP-клиента
tcpclient: tcpclient.c
	$(CC) $(CFLAGS) tcpclient.c -o tcpclient

# Правило для UDP-сервера
udpserver: udpserver.c
	$(CC) $(CFLAGS) udpserver.c -o udpserver

# Правило для UDP-клиента
udpclient: udpclient.c
	$(CC) $(CFLAGS) udpclient.c -o udpclient

# Цель для очистки скомпилированных файлов
clean:
	rm -f $(TARGETS)