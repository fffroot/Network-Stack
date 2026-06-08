# Packet Sniffer - Анализатор Ethernet/IP/TCP заголовков на C

Низкоуровневый сетевой анализатор пакетов, написанный на C с использованием raw sockets. Перехватывает сырые Ethernet-фреймы, разбирает заголовки протоколов (Ethernet, IPv4, TCP) и извлекает HTTP payload в реальном времени.

## 🚀 Возможности

### Перехват трафика
- **Raw Sockets** (`AF_PACKET` + `SOCK_RAW`) — доступ к пакетам на канальном уровне (Layer 2)
- Захват **всех протоколов** через `ETH_P_ALL`
- Бесконечный цикл перехвата с минимальными задержками

### Разбор протоколов
- **Ethernet** (14 байт): MAC-адреса источника/назначения, EtherType
- **IPv4** (20+ байт): Version, IHL, Total Length, TTL, Protocol, Source/Destination IP
- **TCP** (20+ байт): Source/Destination Port, Sequence/Ack Numbers, Flags (SYN/ACK/FIN/RST/PSH/URG), Window Size

### Интеллектуальная фильтрация
- Guard clauses для раннего выхода (early exit pattern)
- Фильтр по протоколам (только IPv4 + TCP)
- Фильтр по портам (HTTP: 80, HTTPS: 443)
- Защита от выхода за границы буфера

### Анализ HTTP трафика
- Извлечение **HTTP payload** (GET/POST запросы)
- Парсинг заголовка `Host:` из HTTP-запросов
- Визуализация payload в ASCII (непечатные символы заменяются на `.`)

### Hex Dump
- Классический формат вывода (как в Wireshark/tcpdump)
- 16 байт на строку с группировкой
- Смещение в шестнадцатеричном виде (`0000:`, `0010:`, ...)
- Hex-представление + ASCII-представление
- Ограничение вывода (первые 64 байта по умолчанию)

## 📋 Требования

- **ОС:** Linux (используется `AF_PACKET`, который специфичен для Linux)
- **Компилятор:** GCC (GNU Compiler Collection)
- **Права:** Root (`sudo`) — raw sockets требуют привилегий
- **Библиотеки:** Только стандартная библиотека C (libc)

## 🛠 Компиляция

### Базовая компиляция
```bash
gcc -Wall -Wextra -o sniffer packet_sniffer.c
```

### С флагами оптимизации
```bash
gcc -O2 -Wall -Wextra -o sniffer packet_sniffer.c
```

### Использование Makefile
```bash
make        # Скомпилировать
make clean  # Удалить исполняемый файл
```

## 💻 Использование

### Запуск сниффера
```bash
sudo ./sniffer
```

Программа выведет:
```
Сниффер запущен. Фильтр: HTTP/HTTPS порты (80, 443)
Жду пакеты...
```

### Генерация тестового трафика

**HTTP запрос (порт 80) — должен появиться:**
```bash
curl http://example.com
```

**HTTPS запрос (порт 443) — должен появиться (но payload зашифрован):**
```bash
curl https://example.com
```

**DNS запрос (порт 53) — НЕ должен появиться:**
```bash
nslookup example.com
```

**ICMP ping — НЕ должен появиться:**
```bash
ping -c 2 example.com
```

## 🐛 Отладка

### Проверка открытых сокетов
```bash
ss -tuln | grep RAW
```

### Мониторинг трафика через tcpdump (для сравнения)
```bash
sudo tcpdump -i any port 80 -vv
```

### Проверка прав
```bash
# Без sudo будет ошибка:
./sniffer
# socket: Operation not permitted

# С sudo работает:
sudo ./sniffer
```

## 📝 Лицензия

MIT License

## 👤 Автор

[fffroot](https://github.com/fffroot)

## 🤝 Вклад в проект

Этот проект является частью обучающего репозитория [Network Stack](https://github.com/fffroot/Network-Stack).