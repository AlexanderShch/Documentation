#include <cstdio>
#include <cstring>
#include <windows.h>
 
#define PORT 666
#define SERVERADDR "127.0.0.1"
#include <winsock2.h>
#include <ws2tcpip.h>

int main(int argc, char* argv[])
{
    printf("TCP DEMO CLIENT\n");
    
    // Шаг 1 - инициализация библиотеки Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        return 1;
    }

    // ...ваш код работы с сокетами...

 
 /*    WSADATA WsaData;
    // Шаг 1 - инициализация библиотеки Winsock
    if(WSAStartup(0x202, &WsaData))
    {
        printf("WSAStart error %d\n", WSAGetLastError());
        return -1;
    }
 */ 
/*     // Шаг 2 - создание сокета
    SOCKET my_sock;
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(my_sock < 0)
    {
        printf("Socket() error %d\n", WSAGetLastError());
        return -1;
    }
 
    // Шаг 3 - установка соединения
    // заполнение структуры sockaddr_in - указание адреса и порта сервера
    sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
 
    // преобразование IP адреса из символьного в сетевой формат
    if(inet_addr(SERVERADDR) != INADDR_NONE)
        dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
    else
    {
        printf("Invalid address %s\n", SERVERADDR);
        closesocket(my_sock);
        WSACleanup();
        return -1;
    }
 
    // адрес сервера получен - пытаемся установить соединение
    if(connect(my_sock, (sockaddr *)&dest_addr, sizeof(dest_addr)))
    {
        printf("Connect error %d\n", WSAGetLastError());
 
        closesocket(my_sock);
        WSACleanup();
        return -1;
    }
 
    printf("Connected to port %d, type 0 for exit\n", PORT);
    int value;
    printf("Enter value: ");
    scanf("%d", &value);
 
    value = htonl(value);
    send(my_sock, (char*)&value, sizeof(value), 0);
 
    // int result;
    recv(my_sock, (char*)&result, sizeof(result), 0);
    result = ntohl(result);
    printf("Computed value [%d]\n", result);
 
    closesocket(my_sock);
 */    
    WSACleanup();
    return 0;
}