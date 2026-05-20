#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>



int main(){
    int sock_fd;
    struct sockaddr_in addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[1024];

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1){
        perror("bind");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    printf("UDP-сервер слушает порт 8080\n");

    while(1){
        ssize_t bytes = recvfrom(sock_fd, buffer, sizeof(buffer) - 1, 0,
                        (struct sockaddr*)&client_addr,&client_len);
        if (bytes == -1){
            perror("recvfrom");
            continue;

        }
        buffer[bytes] = '\0';
        // Печатаем кто прислал и что
        printf("Получено %zd байт от %s:%d: %s\n",
            bytes, inet_ntoa(client_addr.sin_addr),
            ntohs(client_addr.sin_port), buffer);
        const char *response = "Hello from UDP server!\n";
        sendto(sock_fd, response, strlen(response), 0,
           (struct sockaddr*)&client_addr, client_len);

    }
    close(sock_fd);
    return 0;
}