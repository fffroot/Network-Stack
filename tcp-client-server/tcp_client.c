#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from C client!\n";

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        perror("sock failed");
        exit(EXIT_FAILURE);
    }

    printf("✅ Сокет клиента создан\n");

    // 2. Заполняем адрес сервера
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);


    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }

    printf("✅ Адрес сервера подготовлен: %s:%d\n", SERVER_IP, PORT);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }


    printf("🤝 Подключено к серверу!\n");
    if (send(sock, hello, strlen(hello), 0) < 0) {
        perror("send failed");
        exit(EXIT_FAILURE);
    }


    printf("📤 Данные отправлены: %s", hello);

    // 5. Закрываем сокет
    close(sock);
    printf("🔒 Соединение закрыто\n");

    return 0;
}