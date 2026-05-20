#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


int main(){
    int server_fd, client_fd;
    struct sockaddr_in addr;
    char buffer[1024];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    // Очищаем структуру (заполняем нулями)
    memset(&addr, 0, sizeof(addr));
    // Заполняем адрес
    addr.sin_family = AF_INET; //IPv4
    addr.sin_port = htons(8080); //Порт 8080 в сетевом порядке
    addr.sin_addr.s_addr = INADDR_ANY; // Принимать с любых IP (0.0.0.0)

    //    int listen(int sockfd, int backlog);
    //                  ↑ сокет     ↑ очередь ожидания
    // Привязываем сокет к адресу
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr))== -1){
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == -1){
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Сервер слушает порт 8080\n");

    // Бесконечный цикл обработки клиентов
    while (1) {
        socklen_t addrlen = sizeof(addr);
        client_fd = accept(server_fd,(struct sockaddr*)&addr,&addrlen);
        //    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
        //                  ↑ слушающий  ↑ сюда запишет адрес клиента  ↑ размер (указатель!)
        if (client_fd == - 1){
            perror("accept");
            continue; // Не выходим, ждём следующего клиента
        }
        printf("Клиент подключился!\n");

        ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        //        ssize_t recv(int sockfd, void *buf, size_t len, int flags);
        //                          ↑ сокет     ↑ куда    ↑ размер    ↑ флаги (0 = без особенностей)
        if (bytes == -1) {
            perror("recv");
            close(client_fd);
            continue; // Не выходим, ждём следующего клиента
        }
        buffer[bytes] = '\0';  // добавляем конец строки
        printf("Получено %zd байт: %s\n", bytes, buffer);


        //    ssize_t send(int sockfd, const void *buf, size_t len, int flags);
        //                      ↑ сокет      ↑ что          ↑ сколько   ↑ флаги
        const char *response = "Hello from server!\n";
        ssize_t sent = send(client_fd, response, strlen(response), 0);
        if (sent == -1) {
            perror("send");
        } else {
            printf("Отправлено %zd байт\n", sent);
        }

        close(client_fd);
        printf("Ожидание следующего клиента...\n\n");
    }

    close(server_fd);
    printf("Сервер завершил работу.\n");
    return 0;
}