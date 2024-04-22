#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <thread> // For std::thread

#pragma comment(lib, "Ws2_32.lib")

// Function to handle communication with a single client
void HandleClient(SOCKET clientSock) {
    while (true) {


        // Receive message from client
        char recvData[1024];
        int recvSize = recv(clientSock, recvData, sizeof(recvData), 0);
        if (recvSize == SOCKET_ERROR) {
            printf("Client disconnected\n", WSAGetLastError());
            closesocket(clientSock);
            return;
        }
        else if (recvSize == 0) {
            // Connection closed by client
            printf("Client disconnected\n");
            closesocket(clientSock);
            return;
        }

        recvData[recvSize] = '\0';
        printf("Received message from %s\n", recvData);

        // Echo the received message back to the client
        if (send(clientSock, recvData, recvSize, 0) == SOCKET_ERROR) {
            printf("send failed: %d\n", WSAGetLastError());
            closesocket(clientSock);
            return;
        }
    }
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        return 1;
    }

    // Create a socket
    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSock == INVALID_SOCKET) {
        printf("socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Bind the socket
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(7777);
    if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSock, SOMAXCONN) == SOCKET_ERROR) {
        printf("listen failed: %d\n", WSAGetLastError());
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    printf("Server started. Waiting for clients...\n");

    while (true) {
        // Accept client connection
        SOCKET clientSock = accept(serverSock, NULL, NULL);
        if (clientSock == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            closesocket(serverSock);
            WSACleanup();
            return 1;
        }

        printf("client connected\n");


        // Start a new thread to handle communication with the client
        std::thread(HandleClient, clientSock).detach();
    }

    // Close the server socket 
    closesocket(serverSock);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
