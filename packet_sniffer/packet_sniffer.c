#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>



// Заголовок Ethernet (14 байт)
struct eth_hdr {
    unsigned char dest_mac[6];   // MAC-адрес получателя 6 байт
    unsigned char src_mac[6];    // MAC-адрес отправителя 6 байт
    unsigned short type;          // Тип протокола (0x0800 = IPv4, 0x86DD = IPv6) 2 байта
}__attribute__((packed));      //указание компилятору не добавлять пустые байты для выравнивания структур в памяти

// Заголовок IPv4 (обычно 20 байт)
struct ip_hdr {
    unsigned char ver_ihl;      // 4 бита версия + 4 бита длина заголовка
    unsigned char tos;          // Type of Service 1 байт
    unsigned short total_len;   // Длина всего IP-пакета 2 байта
    unsigned short id;          // Идентификатор 2 байта
    unsigned short frag_off;    // Флаги фрагментации 2 байта
    unsigned char ttl;          // Time To Live 1 байт
    unsigned char protocol;     // Протокол верхнего уровня (6 = TCP, 17 = UDP) 1 байт
    unsigned short checksum;    // Контрольная сумма 2 байта
    unsigned int src_ip;        // IP отправителя 4 байта
    unsigned int dst_ip;        // IP получателя 4 байта
}__attribute__((packed));

// Заголовок TCP (обычно 20 байт)
struct tcp_hdr {
    unsigned short src_port;    // Порт отправителя 2 байта
    unsigned short dst_port;    // Порт получателя 2 байта
    unsigned int seq_num;       // Порядковый номер 4 байта
    unsigned int ack_num;       // Номер подтверждения 4 байта
    unsigned char offset_res;   // Длина заголовка
    unsigned char flags;        // SYN, ACK, FIN, RST и т.д.
    unsigned short window;      // Размер окна 2 байта
    unsigned short checksum;    // Контрольная сумма
    unsigned short urgent;      // Указатель срочности

}__attribute__((packed));


/**
 * Функция печатает дамп памяти в формате hex + ASCII
 * @param buffer     - указатель на данные (массив байт)
 * @param len        - реальная длина данных в буфере
 * @param max_bytes  - максимальное количество байт для вывода (чтобы не залить экран)
 *
 * Пример вывода:
 * 0000: 48 65 6c 6c 6f 20 77 6f  72 6c 64 21 00 00 00 00   Hello world!....
 */
void print_hex_dump(unsigned char *buffer, int len, int max_bytes) {
    // Определяем, сколько байт реально будем печатать:
    // - не больше max_bytes (ограничение на вывод)
    // - и не больше len (чтобы не выйти за границы буфера)
    int limit = (len < max_bytes) ? len : max_bytes;

    printf("🔍 Hex Dump (first %d bytes):\n", limit);

    // Внешний цикл: идем по строкам (каждая строка = 16 байт)
    // i - это смещение (адрес) первого байта в текущей строке
    for (int i = 0; i < limit; i += 16) {
        // Печатаем адрес (смещение) в шестнадцатеричном виде, 4 цифры с ведущими нулями
        printf("%04x: ", i);

        // ========== ЧАСТЬ 1: HEX-представление ==========
        // Внутренний цикл: печатаем 16 столбцов hex-значений
        for (int j = 0; j < 16; j++) {
            // Проверяем: не вышли ли мы за пределы limit
            if (i + j < limit) {
                // Печатаем байт как два шестнадцатеричных символа с ведущим нулем
                printf("%02X ", buffer[i + j]);
            } else {
                // Если байта нет (последняя строка неполная) — печатаем 3 пробела
                // Это сохраняет выравнивание колонок
                printf("   ");
            }

            // После 8-го байта (j == 7) добавляем дополнительный пробел
            // Это визуально разделяет строку на две половины по 8 байт
            if (j == 7) {
                printf(" ");
            }
        }

        // Разделитель между hex-блоком и ASCII-блоком
        printf("  ");

        // ========== ЧАСТЬ 2: ASCII-представление ==========
        // Второй внутренний цикл: снова пробегаем те же 16 байт
        for (int j = 0; j < 16; j++) {
            if (i + j < limit) {
                unsigned char c = buffer[i + j];

                // Проверяем, является ли символ печатным:
                // 32 = пробел (' '), 126 = тильда ('~')
                // Все, что вне этого диапазона — управляющие символы или не-ASCII
                if (c >= 32 && c <= 126) {
                    printf("%c", c);  // Печатаем сам символ
                } else {
                    printf(".");       // Непечатный байт заменяем на точку
                }
            }
            // Если байта нет — ничего не печатаем (оставляем пустое место)
        }

        // Переход на новую строку после обработки 16 байт
        printf("\n");
    }

    // Пустая строка для визуального отделения от следующего вывода
    printf("\n");
}

int main() {
    // ==========================================
    // 1. СОЗДАНИЕ СЫРОГО СОКЕТА
    // ==========================================
    // AF_PACKET - работа на канальном уровне (Ethernet)
    // SOCK_RAW  - сырые пакеты с заголовками
    // htons(ETH_P_ALL) - принимаем ВСЕ протоколы (EtherType)
    int raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_socket == -1) {
        perror("socket");  // Вывод ошибки: "Operation not permitted" если не root
        exit(EXIT_FAILURE);
    }

    printf("Сниффер запущен. Фильтр: HTTP/HTTPS порты (80, 443)\n");
    printf("Жду пакеты...\n\n");

    // Буфер для хранения сырого пакета (максимальный размер Ethernet кадра = 65536)
    unsigned char buffer[65536];

    // ==========================================
    // 2. ГЛАВНЫЙ ЦИКЛ ПЕРЕХВАТА ПАКЕТОВ
    // ==========================================
    while (1) {
        // recv() блокирует программу до прихода следующего пакета
        // len - реальное количество полученных байт
        int len = recv(raw_socket, buffer, sizeof(buffer), 0);
        if (len <= 0) {
            perror("recv");
            continue;  // При ошибке просто ждем следующий пакет
        }

        // ==========================================
        // 3. ЭТАП ФИЛЬТРАЦИИ (Guard Clauses)
        // Каждая проверка "пропускает" неподходящие пакеты
        // ==========================================

        // Проверка 1: Минимальная длина Ethernet кадра (14 байт заголовка)
        if (len < 14) continue;

        // Указатель на Ethernet заголовок (начало буфера)
        struct eth_hdr *eth = (struct eth_hdr *)buffer;
        // ntohs() - преобразуем из сетевого порядка байт в порядок хоста
        unsigned short eth_type = ntohs(eth->type);

        // Пропускаем всё, что НЕ IPv4 (EtherType 0x0800)
        if (eth_type != 0x0800) continue;

        // Проверка 2: Минимальная длина IP заголовка (20 байт)
        if (len < 14 + 20) continue;

        // Указатель на IP заголовок (сразу после Ethernet: смещение 14)
        struct ip_hdr *ip = (struct ip_hdr *)(buffer + 14);

        // Извлекаем длину IP заголовка (младшие 4 бита первого байта)
        // IHL = Internet Header Length, в 32-битных словах
        unsigned char ihl = ip->ver_ihl & 0x0F;
        int ip_header_len = ihl * 4;  // Переводим в байты

        // Пропускаем всё, что НЕ TCP (protocol 6)
        if (ip->protocol != 6) continue;

        // Проверка 3: Минимальная длина TCP заголовка (20 байт)
        if (len < 14 + ip_header_len + 20) continue;

        // Указатель на TCP заголовок (IP заголовок может быть разной длины)
        struct tcp_hdr *tcp = (struct tcp_hdr *)(buffer + 14 + ip_header_len);

        // Извлекаем порты (преобразуем из сетевого порядка)
        int src_port = ntohs(tcp->src_port);
        int dst_port = ntohs(tcp->dst_port);

        // Главный фильтр: показываем ТОЛЬКО HTTP (80) и HTTPS (443)
        // Проверяем оба направления: от клиента к серверу и обратно
        if (src_port != 80 && src_port != 443 &&
            dst_port != 80 && dst_port != 443) {
            continue;  // Не наш порт - тихо пропускаем
        }

        // ==========================================
        // 4. ВЫВОД ИНФОРМАЦИИ О ПАКЕТЕ
        // (Сюда попадают ТОЛЬКО HTTP/HTTPS пакеты)
        // ==========================================

        printf("🎯 Найден HTTP/HTTPS пакет! (%d байт)\n", len);

        // 4.1. HEX дамп (первые 64 байта для отладки)
        print_hex_dump(buffer, len, 64);

        // 4.2. MAC-адреса (6 байт каждый)
        printf("📡 MAC: %02X:%02X:%02X:%02X:%02X:%02X -> %02X:%02X:%02X:%02X:%02X:%02X\n",
               eth->src_mac[0], eth->src_mac[1], eth->src_mac[2],
               eth->src_mac[3], eth->src_mac[4], eth->src_mac[5],
               eth->dest_mac[0], eth->dest_mac[1], eth->dest_mac[2],
               eth->dest_mac[3], eth->dest_mac[4], eth->dest_mac[5]);

        // 4.3. IP-адреса (преобразуем из 32-битного числа в точечную нотацию)
        struct in_addr src_ip, dst_ip;
        src_ip.s_addr = ip->src_ip;
        dst_ip.s_addr = ip->dst_ip;
        printf("🌐 IP: %s -> %s (TTL: %d)\n",
               inet_ntoa(src_ip), inet_ntoa(dst_ip), ip->ttl);

        // 4.4. TCP информация
        printf("🔌 TCP: Port %d -> %d\n", src_port, dst_port);
        printf("   Seq: %u, Ack: %u\n", ntohl(tcp->seq_num), ntohl(tcp->ack_num));

        // 4.5. TCP Флаги (битовая маска)
        printf("   Flags: ");
        if (tcp->flags & 0x02) printf("SYN ");  // 00000010 - установка соединения
        if (tcp->flags & 0x10) printf("ACK ");  // 00010000 - подтверждение
        if (tcp->flags & 0x01) printf("FIN ");  // 00000001 - закрытие соединения
        if (tcp->flags & 0x08) printf("PSH ");  // 00001000 - немедленная отправка
        if (tcp->flags & 0x04) printf("RST ");  // 00000100 - сброс соединения
        if (tcp->flags & 0x20) printf("URG ");  // 00100000 - срочные данные
        printf("\n");

        printf("   Window: %d\n\n", ntohs(tcp->window));  // Размер окна (управление потоком)

        // ==========================================
        // 5. ИЗВЛЕЧЕНИЕ HTTP PAYLOAD (только для порта 80)
        // ==========================================
        if (src_port == 80 || dst_port == 80) {
            // Вычисляем длину TCP заголовка (с учетом опций)
            // offset_res: старшие 4 бита = длина заголовка в 32-битных словах
            int tcp_header_len = ((tcp->offset_res >> 4) & 0x0F) * 4;

            // Payload начинается после Ethernet + IP + TCP заголовков
            unsigned char *payload = buffer + 14 + ip_header_len + tcp_header_len;

            // Длина payload = общая длина - все заголовки
            int payload_len = len - (14 + ip_header_len + tcp_header_len);

            if (payload_len > 0) {
                printf("📦 HTTP Payload (%d bytes):\n", payload_len);

                // Печатаем не более 200 байт (чтобы не залить экран)
                int show_len = (payload_len < 200) ? payload_len : 200;
                for (int i = 0; i < show_len; i++) {
                    if (payload[i] >= 32 && payload[i] <= 126) {
                        printf("%c", payload[i]);  // Печатные символы как есть
                    } else {
                        printf(".");  // Непечатные заменяем точкой
                    }
                }
                printf("\n");

                // Поиск HTTP метода (GET, POST и т.д.)
                if (strstr((char*)payload, "GET ") || strstr((char*)payload, "POST ")) {
                    printf("🎯 HTTP REQUEST detected!\n");

                    // Извлекаем заголовок Host: (имя сервера)
                    char *host_start = strstr((char*)payload, "Host: ");
                    if (host_start) {
                        host_start += 6;  // Пропускаем слово "Host: "
                        char *host_end = strstr(host_start, "\r\n");  // Ищем конец строки
                        if (host_end) {
                            int host_len = host_end - host_start;  // Длина имени хоста
                            printf("   Host: %.*s\n", host_len, host_start);
                        }
                    }
                }
                printf("\n");
            }
        }

        // Разделитель между пакетами для читаемости
        printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
    }

    // Закрываем сокет (сюда программа дойдет только при завершении)
    close(raw_socket);
    return 0;
}