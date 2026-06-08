CC = gcc
CFLAGS = -Wall -Wextra
TARGET = sniffer

all: $(TARGET)

$(TARGET): packet_sniffer.c
    $(CC) $(CFLAGS) -o $(TARGET) packet_sniffer.c

clean:
    rm -f $(TARGET)

.PHONY: all clean