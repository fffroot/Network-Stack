#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define PORT 8080
#define BACKLOG 10 //Максимальная длина очереди ожидающих подключений


int main(){
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("✅ Сокет успешно создан\n");
        // 2. Устанавливаем опцию SO_REUSEADDR
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    printf("✅ Опция SO_REUSEADDR установлена\n");

    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);  // Конвертируем порт в сетевой порядок!
    address.sin_addr.s_addr = htonl(INADDR_ANY);  // Конвертируем IP в сетевой порядок!

    printf("✅ Структура адреса заполнена\n");

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }


    printf("✅ Сокет успешно привязан к порту %d\n", PORT);

    // 5. Начинаем прослушивать входящие соединения (Listen)
    if (listen(server_fd, BACKLOG) < 0){
        perror("listen failed");
        exit(EXIT_FAILURE);
    }


    printf("👂 Сервер слушает входящие соединения...\n");
    while(1){
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0){
            perror("accept failed");
            exit(EXIT_FAILURE);
        }


        printf("🤝 Клиент подключился!\n");

        printf("IP клиента: %s, Порт: %d\n",
               inet_ntoa(address.sin_addr),
               ntohs(address.sin_port));


        char buffer[1024] = {0};
        int bytes_read;

        bytes_read = recv(new_socket, buffer, sizeof(buffer), 0);

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';  // Завершающий нулевой байт для строки
            printf("📨 Получено от клиента: %s", buffer);
        } else if (bytes_read == 0) {
            printf("👋 Клиент закрыл соединение\n");
        } else {
            perror("recv failed");
        }

        // 8. Закрываем сокет клиента
        close(new_socket);
        printf("🔒 Соединение с клиентом закрыто\n");
    }
    // 9. Закрываем слушающий сокет сервера
    close(server_fd);
    printf("🛑 Сервер остановлен\n");



    return 0;
}