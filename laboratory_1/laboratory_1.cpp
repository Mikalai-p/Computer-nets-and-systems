#include <winsock2.h>
#include <iostream>
#include <cstring>

#pragma comment(lib, "ws2_32.lib")

int main() {
    
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup failed\n";
        return 1;
    }
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        std::cout << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }
    
    sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(1280);
    local.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
        std::cout << "Bind failed\n";
        closesocket(s);
        WSACleanup();
        return 1;
    }

    if (listen(s, 5) == SOCKET_ERROR) {
        std::cout << "Listen failed\n";
        closesocket(s);
        WSACleanup();
        return 1;
    }

    std::cout << "Server started. Waiting for connections...\n";

    while (true) {
        sockaddr_in remote_addr;
        int size = sizeof(remote_addr);
        SOCKET s2 = accept(s, (sockaddr*)&remote_addr, &size);

        if (s2 == INVALID_SOCKET) {
            std::cout << "Accept failed\n";
            continue;
        }

        char word[256];
        int bytesReceived;

        
        bytesReceived = recv(s2, word, sizeof(word) - 1, 0);
        if (bytesReceived > 0) {
            word[bytesReceived] = '\0'; 

            
            int left = 0;
            int right = strlen(word) - 1; 
            bool isPalindrome = true;

            while (left < right) {
                if (word[left] != word[right]) {
                    isPalindrome = false;
                    break;
                }
                left++;
                right--;
            }

            
            const char* response = isPalindrome ? "YES" : "NO";
            send(s2, response, strlen(response), 0);
        }

        closesocket(s2);
    }

    
    closesocket(s);
    WSACleanup();
    return 0;
}