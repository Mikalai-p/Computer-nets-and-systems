#include <winsock2.h>
#include <iostream>
#include <vector>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct Book {
    int id;
    string author;
    string title;
    int year;
    string publisher;
    int pages;
};

vector<Book> books = {
    {1, "Tolstoy", "War and Peace", 1869, "Russian Herald", 1225},
    {2, "Dostoevsky", "Crime and Punishment", 1866, "Russian Herald", 672},
    {3, "Tolstoy", "Anna Karenina", 1877, "Russian Herald", 864},
    {4, "Gogol", "Dead Souls", 1842, "Contemporary", 352},
    {5, "Pushkin", "Eugene Onegin", 1833, "Literary Gazette", 240},
    {6, "Hemingway", "The Old Man and the Sea", 1952, "Scribner", 127},
    {7, "Orwell", "1984", 1949, "Secker & Warburg", 328}
};

DWORD WINAPI ThreadFunc(LPVOID client_socket) {
    SOCKET s2 = *((SOCKET*)client_socket);
    char buf[100];

    while (recv(s2, buf, sizeof(buf), 0)) {
        string author_name(buf);
        string response;

        // Search books by author
        for (const auto& book : books) {
            if (book.author == author_name) {
                response += "ID: " + to_string(book.id) +
                    ", Title: " + book.title +
                    ", Year: " + to_string(book.year) +
                    ", Publisher: " + book.publisher +
                    ", Pages: " + to_string(book.pages) + "\n";
            }
        }

        if (response.empty()) {
            response = "No books found for author: " + author_name + "\n";
        }

        send(s2, response.c_str(), response.length(), 0);
    }

    closesocket(s2);
    return 0;
}

int main() {
    WSADATA wsaData;
    int err;

    WORD wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        cout << "WSAStartup failed" << endl;
        return 1;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(1280);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (sockaddr*)&local_addr, sizeof(local_addr)) == SOCKET_ERROR) {
        cout << "Bind failed" << endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }

    if (listen(s, 5) == SOCKET_ERROR) {
        cout << "Listen failed" << endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }

    cout << "Server started. Waiting for connections..." << endl;

    int numcl = 0;

    while (true) {
        SOCKET client_socket;
        sockaddr_in client_addr;
        int client_addr_size = sizeof(client_addr);

        client_socket = accept(s, (sockaddr*)&client_addr, &client_addr_size);
        if (client_socket == INVALID_SOCKET) {
            cout << "Accept failed" << endl;
            continue;
        }

        numcl++;
        if (numcl) {
            cout << numcl << " client connected" << endl;
        }

        DWORD thID;
        CreateThread(NULL, NULL, ThreadFunc, &client_socket, NULL, &thID);
    }

    closesocket(s);
    WSACleanup();
    return 0;
}