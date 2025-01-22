#include <cstdio>
#include <winsock2.h>
#include <ws2tcpip.h>

// Не забудьте добавить библиотеку Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

int main() { 
    WSADATA wsaData;
    int result;

    // Инициализация Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        return 1;
    }

    // Ваш код работы с сокетами здесь

    // Завершение работы с Winsock
    WSACleanup();

    return 0;
}