#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        return 1;
    }

    // Create a socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Connect to server
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // Convert IPv4 address string to binary form
    serverAddr.sin_port = htons(7777); // Server port
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("connect failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    else {
        printf("Connected to server \n");
    }

    while (true) {
        // Send data
        printf("Enter text to send to the server: ");
        char sendData[1024];
        fgets(sendData, sizeof(sendData), stdin);

        // Remove newline character from the input
        size_t len = strlen(sendData);
        if (sendData[len - 1] == '\n') {
            sendData[len - 1] = '\0';
        }

        if (send(sock, sendData, strlen(sendData), 0) == SOCKET_ERROR) {
            printf("send failed: %d\n", WSAGetLastError());
            closesocket(sock);
            WSACleanup();
            return 1;
        }
        else {
            printf("Data sent to server \n");
        }
    }

    // Close the socket
    closesocket(sock);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
