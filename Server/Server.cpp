#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <thread> // For std::thread
#include <iostream>
#include <fstream> // For file operations
#include <string>

#pragma comment(lib, "Ws2_32.lib")

SOCKET clientSock;
std::string username;

std::string encryptCaesarCipher(const std::string& message, int shift) {
    std::string encryptedMessage = "";
    for (char c : message) {
        if (isalpha(c)) {
            char shifted = c + shift;
            if (islower(c) && shifted > 'z') {
                encryptedMessage += 'a' + (shifted - 'z' - 1);
            }
            else if (isupper(c) && shifted > 'Z') {
                encryptedMessage += 'A' + (shifted - 'Z' - 1);
            }
            else {
                encryptedMessage += shifted;
            }
        }
        else {
            encryptedMessage += c;
        }
    }
    return encryptedMessage;
}
std::string caesarDecrypt(const std::string& cipherText, int shift) {
    std::string decryptedText = "";

    for (char c : cipherText) {
        // Decrypt uppercase letters
        if (isupper(c)) {
            decryptedText += char(int(c - 'A' - shift + 26) % 26 + 'A');
        }
        // Decrypt lowercase letters
        else if (islower(c)) {
            decryptedText += char(int(c - 'a' - shift + 26) % 26 + 'a');
        }
        // Leave non-alphabetic characters unchanged
        else {
            decryptedText += c;
        }
    }

    return decryptedText;
}

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
        std::string decryptedData = caesarDecrypt(recvData, 5);
        printf("Received message from %s\n", decryptedData.c_str(), "\n");
        

    }
}


// Function to sign up a new user
bool SignUp(SOCKET sock) {
    char tempUsername[1024];
    char password[1024];

    printf("Enter a username: ");
    fgets(tempUsername, sizeof(tempUsername), stdin);
    printf("Enter a password: ");
    fgets(password, sizeof(password), stdin);

    // Remove newline characters from input
    size_t len_user = strlen(tempUsername);
    if (tempUsername[len_user - 1] == '\n') {
        tempUsername[len_user - 1] = '\0';
    }
    size_t len_pass = strlen(password);
    if (password[len_pass - 1] == '\n') {
        password[len_pass - 1] = '\0';
    }

    // Open the user file to check for duplicates
    std::ifstream userFile("server_user_info.txt");
    std::string line;
    while (std::getline(userFile, line)) {
        if (line.find(tempUsername) != std::string::npos) {
            printf("Username already exists. Please choose another one.\n");
            return false;
        }
    }
    userFile.close();


    // Save the username and password to the file
    std::ofstream outFile("server_user_info.txt", std::ios::app);
    if (outFile.is_open()) {
        std::string encryptedPass = encryptCaesarCipher(password, 5);
        outFile << tempUsername << " " << encryptedPass << std::endl;
        outFile.close();
        username = tempUsername;
        printf("Sign up successful.\n");
        std::cout << "Enter messages to send to client: \n";
        return true;
    }
    else {
        printf("Failed to open user info file.\n");
        return false;
    }
}

// Function to login an existing user
bool Login(SOCKET sock) {
    char tempUsername[1024];
    char password[1024];

    printf("Enter your username: ");
    fgets(tempUsername, sizeof(tempUsername), stdin);
    printf("Enter your password: ");
    fgets(password, sizeof(password), stdin);

    // Remove newline characters from input
    size_t len_user = strlen(tempUsername);
    if (tempUsername[len_user - 1] == '\n') {
        tempUsername[len_user - 1] = '\0';
    }
    size_t len_pass = strlen(password);
    if (password[len_pass - 1] == '\n') {
        password[len_pass - 1] = '\0';
    }

    // Open the user file to check for matching credentials
    std::ifstream userFile("server_user_info.txt");
    std::string line;
    bool found = false;
    while (std::getline(userFile, line)) {
        std::string encryptedPassw = encryptCaesarCipher(password, 5);
        if (line.find(tempUsername) != std::string::npos && line.find(encryptedPassw) != std::string::npos) {
            found = true;
            break;
        }
    }
    userFile.close();

    // If matching credentials found, return true; otherwise, return false
    if (found) {
        username = tempUsername;
        printf("Login successful.\n");
        std::cout << "Enter message to send to client: \n";
        return true;
    }
    else {
        printf("Login failed. Invalid username or password.\n");
        return false;
    }
}

int main() {
    
    bool loggedIn = false;
    while (!loggedIn) {
        // Ask the client to sign up or login
        printf("1. Sign up\n2. Login\nChoose an option: ");
        int choice;
        scanf_s("%d", &choice);
        getchar(); // Consume the newline character

        if (choice == 1) {
            loggedIn = SignUp(clientSock);
        }
        else if (choice == 2) {
            loggedIn = Login(clientSock);
        }
        else {
            printf("Invalid choice.\n");
        }
    }

    while (loggedIn) {

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



        // Accept client connection
        SOCKET clientSock = accept(serverSock, NULL, NULL);
        if (clientSock == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            closesocket(serverSock);
            WSACleanup();
            return 1;
        }
        printf("Client connected \n");
        // Start a new thread to handle communication with the client
        std::thread(HandleClient, clientSock).detach();

        // Main loop to continuously send messages to the client
        while (true) {
            // Send message to client
            std::string message;
            std::getline(std::cin, message);
            std::string messageUsername = username + ": " + message;
            std::string encryptedData = encryptCaesarCipher(messageUsername, 5);
            if (send(clientSock, encryptedData.c_str(), encryptedData.size(), 0) == SOCKET_ERROR) {
                printf("Error sending message to client\n");
                closesocket(clientSock);
                break;
            }
            else {
                printf("Message sent to client\n");
            }

        }
        // Close the server socket 
        closesocket(serverSock);
    }




    // Cleanup Winsock
    WSACleanup();

    return 0;
}
