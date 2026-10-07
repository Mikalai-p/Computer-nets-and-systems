#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class UDPServer {
private:
    WSADATA wsaData;
    SOCKET serverSocket;
    sockaddr_in serverAddr, clientAddr;
    int clientAddrSize;
    char buffer[1024];

    void initializeWinsock() {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
    }

    void createSocket() {
        serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (serverSocket == INVALID_SOCKET) {
            throw std::runtime_error("Socket creation failed");
        }
    }

    void setupServerAddress() {
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(8888);
    }

    void bindSocket() {
        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            throw std::runtime_error("Bind failed");
        }
    }

    std::string processString(const std::string& input) {
        std::string result = input;
        size_t length = input.length();

        if (length % 4 == 0 && length > 0) {
            size_t mid = length / 2;
            std::string firstPart = result.substr(0, mid);
            std::string secondPart = result.substr(mid);
            result = secondPart + firstPart;
        }

        return result;
    }

public:
    UDPServer() : clientAddrSize(sizeof(clientAddr)) {
        initializeWinsock();
        createSocket();
        setupServerAddress();
        bindSocket();
        std::cout << "UDP Server started on port 8888" << std::endl;
    }

    ~UDPServer() {
        closesocket(serverSocket);
        WSACleanup();
    }

    void run() {
        while (true) {
            std::cout << "Waiting for data..." << std::endl;

            int bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer), 0,
                (sockaddr*)&clientAddr, &clientAddrSize);

            if (bytesReceived == SOCKET_ERROR) {
                std::cerr << "recvfrom failed: " << WSAGetLastError() << std::endl;
                continue;
            }

            std::string receivedData(buffer, bytesReceived);
            std::cout << "Received: " << receivedData << " (length: " << receivedData.length() << ")" << std::endl;

            std::string processedData = processString(receivedData);

            int sendResult = sendto(serverSocket, processedData.c_str(), processedData.length(), 0,
                (sockaddr*)&clientAddr, clientAddrSize);

            if (sendResult == SOCKET_ERROR) {
                std::cerr << "sendto failed: " << WSAGetLastError() << std::endl;
            }
            else {
                std::cout << "Sent: " << processedData << std::endl;
            }
        }
    }
};

int main() {
    try {
        UDPServer server;
        server.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}