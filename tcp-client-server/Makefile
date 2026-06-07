all: server client

server: tcp_server.c
    gcc -Wall -Wextra -o server tcp_server.c

client: tcp_client.c
    gcc -Wall -Wextra -o client tcp_client.c

clean:
    rm -f server client