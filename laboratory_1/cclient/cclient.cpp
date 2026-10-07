#include <winsock2.h>
#include <ws2tcpip.h> 
#include <iostream>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup failed\n";
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cout << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1280);

    
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        std::cout << "Invalid address or address not supported\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "Connection failed: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    char word[256];
    std::cout << "Enter word to check: ";
    std::cin.getline(word, sizeof(word));

    int wordLength = static_cast<int>(strlen(word));
    if (send(clientSocket, word, wordLength, 0) == SOCKET_ERROR) {
        std::cout << "Send failed: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    char response[10];
    int bytesReceived = recv(clientSocket, response, sizeof(response) - 1, 0);
    if (bytesReceived == SOCKET_ERROR) {
        std::cout << "Receive failed: " << WSAGetLastError() << std::endl;
    }
    else {
        response[bytesReceived] = '\0'; 
        std::cout << "Is palindrome: " << response << std::endl;
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}