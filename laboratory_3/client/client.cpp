#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class LibraryClient {
private:
    SOCKET clientSocket;
    sockaddr_in serverAddr;

public:
    LibraryClient() : clientSocket(INVALID_SOCKET) {}

    ~LibraryClient() {
        disconnect();
    }

    bool connectToServer(const std::string& ip = "127.0.0.1", int port = 12345) {
        // Инициализация Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Winsock initialization failed\n";
            return false;
        }

        // Создание сокета
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed\n";
            WSACleanup();
            return false;
        }

        // Настройка адреса сервера
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

        // Подключение к серверу
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Connection to server failed\n";
            closesocket(clientSocket);
            WSACleanup();
            return false;
        }

        std::cout << "Connected to library server at " << ip << ":" << port << std::endl;
        return true;
    }

    void disconnect() {
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
        }
        WSACleanup();
    }

    std::string receiveData() {
        char buffer[4096];
        std::string data;
        int bytesReceived;

        // Установка таймаута на прием данных
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

        while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[bytesReceived] = '\0';
            data += buffer;

            // Проверяем, содержит ли полученная данных маркер конца сообщения
            if (data.find("Enter command: ") != std::string::npos) {
                break;
            }
        }

        if (bytesReceived == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error != WSAETIMEDOUT) {
                std::cerr << "Receive error: " << error << std::endl;
            }
        }

        return data;
    }

    bool sendCommand(const std::string& command) {
        if (send(clientSocket, command.c_str(), command.length(), 0) == SOCKET_ERROR) {
            std::cerr << "Send failed\n";
            return false;
        }
        return true;
    }

    void interactiveMode() {
        std::string command;

        // Получаем приветственное сообщение от сервера
        std::string welcome = receiveData();
        std::cout << welcome;

        while (true) {
            // Чтение команды от пользователя
            std::cout << "> ";
            std::getline(std::cin, command);

            if (command.empty()) {
                continue;
            }

            // Добавляем перевод строки для сервера
            std::string fullCommand = command + "\n";

            // Отправка команды
            if (!sendCommand(fullCommand)) {
                break;
            }

            // Получение ответа от сервера
            std::string response = receiveData();
            std::cout << response;

            // Выход из цикла если команда EXIT
            if (command == "EXIT") {
                break;
            }
        }
    }

    void run() {
        if (!connectToServer()) {
            return;
        }

        interactiveMode();
        disconnect();
    }
};

void printHelp() {
    std::cout << "\n=== LIBRARY CLIENT HELP ===" << std::endl;
    std::cout << "Available commands:" << std::endl;
    std::cout << "SEARCH <author>     - Find books by author" << std::endl;
    std::cout << "ALL                 - Show all books in library" << std::endl;
    std::cout << "ADD parameters      - Add new book" << std::endl;
    std::cout << "EDIT parameters     - Edit book information" << std::endl;
    std::cout << "DELETE <number>     - Delete book by registration number" << std::endl;
    std::cout << "EXIT                - Exit client" << std::endl;
    std::cout << "\nAdd/Edit format:" << std::endl;
    std::cout << "ADD reg_number,author,title,year,publisher,pages" << std::endl;
    std::cout << "EDIT reg_number,author,title,year,publisher,pages" << std::endl;
    std::cout << "\nExamples:" << std::endl;
    std::cout << "SEARCH Tolstoy" << std::endl;
    std::cout << "ADD 8,Chekhov,The Cherry Orchard,1904,Znanie,96" << std::endl;
    std::cout << "EDIT 1,Tolstoy,War and Peace,1869,Russian Messenger,1225" << std::endl;
    std::cout << "DELETE 3" << std::endl;
    std::cout << "===========================\n" << std::endl;
}

int main() {
    std::cout << "Library Management System Client" << std::endl;
    std::cout << "Connecting to server..." << std::endl;

    printHelp();

    LibraryClient client;
    client.run();

    std::cout << "Client stopped. Press Enter to exit...";
    std::cin.get();

    return 0;
}