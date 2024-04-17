#include <WinSock2.h> // Include Winsock header
#include <WS2tcpip.h> // Include for getaddrinfo function
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")


int main() 
{
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        return 1;
    }
    else
    {
        printf("Successful Winsock Initialization \n");
    }

    // Create a socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    else
    {
        printf("Successful Socket Creation \n");
    }

    // Bind a local address to the socket
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(7777);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    else
    {
        printf("Successful Binding Local Address to Socket \n");
    }

    // Listen for incoming connections
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
        printf("listen failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    else
    {
        printf("Listening... \n");
    }

    // Accept a connection
    SOCKET clientSock = accept(sock, NULL, NULL);
    if (clientSock == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    else
    {
        printf("Accepted Connection \n");
    }

    while (true) {


        // Receive data
        char recvData[1024];
        int recvSize = recv(clientSock, recvData, sizeof(recvData), 0);
        if (recvSize == SOCKET_ERROR) {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(clientSock);
            closesocket(sock);
            WSACleanup();
            return 1;
        }
        recvData[recvSize] = '\0';
        printf("Received data: %s\n", recvData);


    }

    // Close the client socket
    closesocket(clientSock);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
