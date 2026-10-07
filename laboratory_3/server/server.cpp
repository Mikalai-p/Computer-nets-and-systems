#include <iostream>
#include <vector>
#include <string>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <algorithm>
#include <sstream>
#include <stdexcept>

#pragma comment(lib, "ws2_32.lib")

struct Book {
    int regNumber;
    std::string author;
    std::string title;
    int year;
    std::string publisher;
    int pages;
};


struct ThreadData {
    void* server;
    SOCKET clientSocket;
};

class LibraryServer {
private:
    std::vector<Book> books;
    std::mutex booksMutex;
    SOCKET serverSocket;
    bool isRunning;

    
    static DWORD WINAPI ThreadFunc(LPVOID lpParam) {
        ThreadData* data = (ThreadData*)lpParam;
        LibraryServer* server = (LibraryServer*)data->server;
        SOCKET clientSocket = data->clientSocket;

        server->handleClient(clientSocket);

        delete data; 
        return 0;
    }

    void initializeBooks() {
        books = {
            {1, "Tolstoy", "War and Peace", 1869, "Russian Messenger", 1225},
            {2, "Dostoevsky", "Crime and Punishment", 1866, "Russian Messenger", 671},
            {3, "Tolstoy", "Anna Karenina", 1877, "Russian Messenger", 864},
            {4, "Gogol", "Dead Souls", 1842, "The Contemporary", 352},
            {5, "Pushkin", "Eugene Onegin", 1833, "Literary Gazette", 240},
            {6, "Dostoevsky", "The Idiot", 1869, "Russian Messenger", 640},
            {7, "Turgenev", "Fathers and Sons", 1862, "Russian Messenger", 288}
        };
    }

    std::string searchBooksByAuthor(const std::string& author) {
        std::lock_guard<std::mutex> lock(booksMutex);
        std::string result;

        for (const auto& book : books) {
            if (book.author == author) {
                result += "Reg. number: " + std::to_string(book.regNumber) + "\n";
                result += "Author: " + book.author + "\n";
                result += "Title: " + book.title + "\n";
                result += "Year: " + std::to_string(book.year) + "\n";
                result += "Publisher: " + book.publisher + "\n";
                result += "Pages: " + std::to_string(book.pages) + "\n";
                result += "------------------------\n";
            }
        }

        if (result.empty()) {
            result = "Books by author '" + author + "' not found.\n";
        }

        return result;
    }

    std::string getAllBooks() {
        std::lock_guard<std::mutex> lock(booksMutex);
        std::string result = "=== ALL BOOKS IN LIBRARY ===\n\n";

        for (const auto& book : books) {
            result += "Reg. number: " + std::to_string(book.regNumber) + "\n";
            result += "Author: " + book.author + "\n";
            result += "Title: " + book.title + "\n";
            result += "Year: " + std::to_string(book.year) + "\n";
            result += "Publisher: " + book.publisher + "\n";
            result += "Pages: " + std::to_string(book.pages) + "\n";
            result += "------------------------\n";
        }

        result += "Total books: " + std::to_string(books.size()) + "\n";
        return result;
    }

    std::string addBook(const std::vector<std::string>& params) {
        if (params.size() < 6) {
            return "Error: not enough parameters. Format: ADD reg_number,author,title,year,publisher,pages\n";
        }

        std::lock_guard<std::mutex> lock(booksMutex);

        try {
            Book newBook;
            newBook.regNumber = std::stoi(params[0]);
            newBook.author = params[1];
            newBook.title = params[2];
            newBook.year = std::stoi(params[3]);
            newBook.publisher = params[4];
            newBook.pages = std::stoi(params[5]);

            for (const auto& book : books) {
                if (book.regNumber == newBook.regNumber) {
                    return "Error: book with registration number " + std::to_string(newBook.regNumber) + " already exists\n";
                }
            }

            books.push_back(newBook);
            return "Book '" + newBook.title + "' successfully added to library!\n";

        }
        catch (const std::exception& e) {
            return "Error: invalid data format. Make sure number, year and pages are numbers\n";
        }
    }

    std::string editBook(const std::vector<std::string>& params) {
        if (params.size() < 6) {
            return "Error: not enough parameters. Format: EDIT reg_number,author,title,year,publisher,pages\n";
        }

        std::lock_guard<std::mutex> lock(booksMutex);

        try {
            int regNumber = std::stoi(params[0]);

            for (auto& book : books) {
                if (book.regNumber == regNumber) {
                    std::string oldTitle = book.title;
                    book.author = params[1];
                    book.title = params[2];
                    book.year = std::stoi(params[3]);
                    book.publisher = params[4];
                    book.pages = std::stoi(params[5]);
                    return "Book '" + oldTitle + "' successfully edited!\n";
                }
            }

            return "Error: book with registration number " + std::to_string(regNumber) + " not found\n";

        }
        catch (const std::exception& e) {
            return "Error: invalid data format. Make sure number, year and pages are numbers\n";
        }
    }

    std::string deleteBook(const std::string& regNumberStr) {
        std::lock_guard<std::mutex> lock(booksMutex);

        try {
            int regNumber = std::stoi(regNumberStr);

            auto it = std::remove_if(books.begin(), books.end(),
                [regNumber](const Book& book) {
                    return book.regNumber == regNumber;
                });

            if (it != books.end()) {
                books.erase(it, books.end());
                return "Book successfully deleted from library!\n";
            }

            return "Error: book with registration number " + regNumberStr + " not found\n";

        }
        catch (const std::exception& e) {
            return "Error: invalid registration number format\n";
        }
    }

    std::vector<std::string> splitString(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);

        while (std::getline(tokenStream, token, delimiter)) {
            if (!token.empty()) {
                tokens.push_back(token);
            }
        }

        return tokens;
    }

    bool sendToClient(SOCKET clientSocket, const std::string& message) {
        int totalSent = 0;
        int messageLength = static_cast<int>(message.length());
        const char* messagePtr = message.c_str();

        while (totalSent < messageLength) {
            int sent = send(clientSocket, messagePtr + totalSent, messageLength - totalSent, 0);
            if (sent == SOCKET_ERROR) {
                return false;
            }
            totalSent += sent;
        }
        return true;
    }

    void handleClient(SOCKET clientSocket) {
        char buffer[1024];
        int bytesReceived = 0;

        std::string welcome =
            "LIBRARY MANAGEMENT SYSTEM\n"
            "=========================\n\n"
            "Available commands:\n"
            "SEARCH <author>     - find books by author\n"
            "ALL                 - show all books\n"
            "ADD <parameters>    - add new book\n"
            "EDIT <parameters>   - edit book data\n"
            "DELETE <number>     - delete book\n"
            "EXIT               - exit\n\n"
            "Add/Edit format:\n"
            "ADD number,author,title,year,publisher,pages\n\n"
            "Enter command: ";

        if (!sendToClient(clientSocket, welcome)) {
            std::cout << "Error sending welcome to client\n";
            closesocket(clientSocket);
            return;
        }

        while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[bytesReceived] = '\0';
            std::string command(buffer);

            // Remove newline characters
            command.erase(std::remove(command.begin(), command.end(), '\n'), command.end());
            command.erase(std::remove(command.begin(), command.end(), '\r'), command.end());

            if (command.empty()) {
                sendToClient(clientSocket, "Enter command: ");
                continue;
            }

            std::string response;

            if (command == "EXIT") {
                response = "Goodbye!\n";
                sendToClient(clientSocket, response);
                break;
            }
            else if (command.length() >= 7 && command.substr(0, 7) == "SEARCH ") {
                std::string author = command.substr(7);
                response = "Search results for author '" + author + "':\n\n";
                response += searchBooksByAuthor(author);
            }
            else if (command == "ALL") {
                response = getAllBooks();
            }
            else if (command.length() >= 4 && command.substr(0, 4) == "ADD ") {
                std::string paramsStr = command.substr(4);
                auto params = splitString(paramsStr, ',');
                response = addBook(params);
            }
            else if (command.length() >= 5 && command.substr(0, 5) == "EDIT ") {
                std::string paramsStr = command.substr(5);
                auto params = splitString(paramsStr, ',');
                response = editBook(params);
            }
            else if (command.length() >= 7 && command.substr(0, 7) == "DELETE ") {
                std::string regNumber = command.substr(7);
                response = deleteBook(regNumber);
            }
            else {
                response = "Unknown command. Use: SEARCH, ALL, ADD, EDIT, DELETE, EXIT\n";
            }

            response += "\nEnter command: ";

            if (!sendToClient(clientSocket, response)) {
                break;
            }
        }

        closesocket(clientSocket);
        std::cout << "Client disconnected\n";
    }

    void cleanup() {
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
        }
        WSACleanup();
    }

public:
    LibraryServer() : serverSocket(INVALID_SOCKET), isRunning(false) {
        initializeBooks();
    }

    ~LibraryServer() {
        stop();
        cleanup();
    }

    bool start(int port = 12345) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "Winsock initialization error\n";
            return false;
        }

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation error\n";
            WSACleanup();
            return false;
        }

        // Set socket option for address reuse
        int opt = 1;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
            std::cerr << "Socket option setting error\n";
            cleanup();
            return false;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Error binding socket to port " << port << std::endl;
            cleanup();
            return false;
        }

        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Port listening error\n";
            cleanup();
            return false;
        }

        std::cout << "Server started on port " << port << std::endl;
        std::cout << "Books loaded: " << books.size() << std::endl;
        std::cout << "Waiting for connections..." << std::endl;

        isRunning = true;

        while (isRunning) {
            sockaddr_in clientAddr;
            int clientAddrSize = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);

            if (clientSocket == INVALID_SOCKET) {
                if (isRunning) {
                    std::cerr << "Connection accept error\n";
                }
                continue;
            }

            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
            std::cout << "New connection from " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;

            
            ThreadData* data = new ThreadData();
            data->server = this;
            data->clientSocket = clientSocket;

            
            DWORD thID;  
            HANDLE hThread = CreateThread(
                NULL,                   
                NULL,                    
                ThreadFunc,             
                (LPVOID)data,          
                NULL,                   
                &thID                  
            );

            if (hThread == NULL) {
                std::cerr << "Error creating thread for client\n";
                closesocket(clientSocket);
                delete data;
            }
            else {
                CloseHandle(hThread); 
            }
        }

        return true;
    }

    void stop() {
        isRunning = false;
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
        }
    }
};

int main() {
    LibraryServer server;
    std::cout << "Starting library server...\n";

    if (!server.start()) {
        std::cerr << "Failed to start server\n";
        return 1;
    }

    std::cout << "Server stopped\n";
    return 0;
}