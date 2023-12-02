
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#include <iostream>
#include <cstring>

const char* SERVER_IP = "127.0.0.1";
const int PORT = 12345;
const int BUFFER_SIZE = 1024;

#ifdef _WIN32
// Преобразование строки в формат PCWSTR
std::wstring ConvertToWideString(const char* narrowString) {
    int length = MultiByteToWideChar(CP_ACP, 0, narrowString, -1, NULL, 0);
    wchar_t* wideString = new wchar_t[length];
    MultiByteToWideChar(CP_ACP, 0, narrowString, -1, wideString, length);
    std::wstring result(wideString);
    delete[] wideString;
    return result;
}
#endif

int main() {
#ifdef _WIN32
    // Инициализация библиотеки сокетов для Windows
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error while initializing the socket library" << std::endl;
        return -1;
    }
#endif

    // Создание сокета
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error preparing socket" << std::endl;
#ifdef _WIN32
        closesocket(clientSocket);
#else
        close(clientSocket);
#endif
        return -1;
    }

    // Заполнение информации о сервере
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

#ifdef _WIN32
    // Для Windows используем InetPton
    std::wstring wideServerIP = ConvertToWideString(SERVER_IP);
    if (InetPton(AF_INET, wideServerIP.c_str(), &(serverAddress.sin_addr)) <= 0) {
        std::cerr << "Error while resolving IP address" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }
#else
    // Для других платформ используем inet_pton
    if (inet_pton(AF_INET, SERVER_IP, &(serverAddress.sin_addr)) <= 0) {
        std::cerr << "Error while resolving IP address" << std::endl;
        close(clientSocket);
        return -1;
    }
#endif

    // Подключение к серверу
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error connecting to server" << std::endl;
#ifdef _WIN32
        closesocket(clientSocket);
#else
        close(clientSocket);
#endif
        return -1;
    }

    std::cout << "Connected to server" << std::endl;

    char buffer[BUFFER_SIZE];
    while (true) {
        std::cout << "Send to server : ";
        std::cin.getline(buffer, BUFFER_SIZE);

        // Отправка данных на сервер
        send(clientSocket, buffer, strlen(buffer), 0);

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // Получение ответа от сервера
        memset(buffer, 0, BUFFER_SIZE);
        recv(clientSocket, buffer, BUFFER_SIZE, 0);
        std::cout << "Received from server:" << buffer << std::endl;
    }

    // Закрытие сокета
#ifdef _WIN32
    closesocket(clientSocket);
    WSACleanup();
#else
    close(clientSocket);
#endif

    return 0;
}