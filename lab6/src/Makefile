CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SERVER_LIBS = -pthread
SERVERS_FILE = servers.txt

CLIENT_TARGET = client
SERVER_TARGET = server

UTILS_SRC = utils.c
UTILS_OBJ = utils.o
UTILS_LIB = libutils.a

# Цель по умолчанию: компилирует все
all: $(UTILS_LIB) $(CLIENT_TARGET) $(SERVER_TARGET)

# Компиляция клиента: линкуем с библиотекой libutils.a
$(CLIENT_TARGET): client.c $(UTILS_LIB)
	$(CC) $(CFLAGS) $< $(UTILS_LIB) -o $@

# Компиляция сервера: линкуем с библиотекой libutils.a
$(SERVER_TARGET): server.c $(UTILS_LIB)
	$(CC) $(CFLAGS) $< $(UTILS_LIB) -o $@ $(SERVER_LIBS)

# Создание статической библиотеки из объектного файла
$(UTILS_LIB): $(UTILS_OBJ)
	ar rcs $@ $<

# Компиляция исходника библиотеки в объектный файл
$(UTILS_OBJ): $(UTILS_SRC) utils.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CLIENT_TARGET) $(SERVER_TARGET) $(SERVERS_FILE) $(UTILS_LIB) $(UTILS_OBJ)


run_servers: $(SERVER_TARGET)
	@echo "127.0.0.1:20001" > $(SERVERS_FILE)
	@echo "127.0.0.1:20002" >> $(SERVERS_FILE)
	./$(SERVER_TARGET) --port 20001 --tnum 2 &
	./$(SERVER_TARGET) --port 20002 --tnum 2 &
	@sleep 1

run_client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET) --k 10 --mod 1000 --servers $(SERVERS_FILE)

test: run_servers run_client
	@killall $(SERVER_TARGET) || true

.PHONY: all clean run_servers run_client test