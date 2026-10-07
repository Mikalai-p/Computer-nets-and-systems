#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class UDPClient {
private:
    WSADATA wsaData;
    SOCKET clientSocket;
    sockaddr_in serverAddr;
    char buffer[1024];

    void initializeWinsock() {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
    }

    void createSocket() {
        clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (clientSocket == INVALID_SOCKET) {
            throw std::runtime_error("Socket creation failed");
        }
    }

    void setupServerAddress() {
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(8888);
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    }

public:
    UDPClient() {
        initializeWinsock();
        createSocket();
        setupServerAddress();
        std::cout << "UDP Client started. Connecting to server on port 8888" << std::endl;
    }

    ~UDPClient() {
        closesocket(clientSocket);
        WSACleanup();
    }

    void sendMessage(const std::string& message) {
        int sendResult = sendto(clientSocket, message.c_str(), message.length(), 0,
            (sockaddr*)&serverAddr, sizeof(serverAddr));

        if (sendResult == SOCKET_ERROR) {
            throw std::runtime_error("Send failed");
        }

        std::cout << "Message sent: " << message << std::endl;

        sockaddr_in fromAddr;
        int fromLen = sizeof(fromAddr);
        int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0,
            (sockaddr*)&fromAddr, &fromLen);

        if (bytesReceived == SOCKET_ERROR) {
            throw std::runtime_error("Receive failed");
        }

        std::string response(buffer, bytesReceived);
        std::cout << "Server response: " << response << std::endl;
    }
};

int main() {
    try {
        UDPClient client;
        std::string input;

        while (true) {
            std::cout << "Enter a string (or 'quit' to exit): ";
            std::getline(std::cin, input);

            if (input == "quit") {
                break;
            }

            if (!input.empty()) {
                client.sendMessage(input);
            }
        }

        std::cout << "Client terminated." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}