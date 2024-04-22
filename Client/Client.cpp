#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <fstream> // For file operations
#include <string>

#pragma comment(lib, "Ws2_32.lib")

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

std::string username;
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
    std::ifstream userFile("/user_info.txt");
    std::string line;
    while (std::getline(userFile, line)) {
        if (line.find(tempUsername) != std::string::npos) {
            printf("Username already exists. Please choose another one.\n");
            return false;
        }
    }
    userFile.close();

    // Send the signup data to the server
    std::string signUpData = std::string(tempUsername) + " Signed Up \n";
    if (send(sock, signUpData.c_str(), signUpData.size(), 0) == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        return false;
    }

    // Save the username and password to the file
    std::ofstream outFile("/user_info.txt", std::ios::app);
    if (outFile.is_open()) {
        outFile << tempUsername << " " << password << std::endl;
        outFile.close();
        username = tempUsername;
        printf("Sign up successful.\n");
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
    // Remove newline character from username
    tempUsername[strcspn(tempUsername, "\n")] = '\0';

    printf("Enter your password: ");
    fgets(password, sizeof(password), stdin);
    // Remove newline character from password
    password[strcspn(password, "\n")] = '\0';

    // Open the user file to check for matching credentials
    std::ifstream userFile("user_info.txt");
    std::string line;
    bool found = false;
    while (std::getline(userFile, line)) {
        if (line.find(tempUsername) != std::string::npos && line.find(password) != std::string::npos) {
            found = true;
            break;
        }
    }
    userFile.close();

    // Send the login data to the server
    std::string loginData = std::string(tempUsername) + " Logged In \n";
    if (send(sock, loginData.c_str(), loginData.size(), 0) == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        return false;
    }

    // If matching credentials found, return true; otherwise, return false
    if (found) {
        username = tempUsername;
        printf("Login successful.\n");
        return true;
    }
    else {
        printf("Login failed. Invalid username or password.\n");
        return false;
        
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
        // Sign up or login
        printf("1. Sign up\n2. Login\nChoose an option: ");
        int choice;
        scanf_s("%d", &choice);
        getchar(); // Consume the newline character
        bool loggedIn = false;

        if (choice == 1) {
            if (SignUp(sock)) {
                loggedIn = true;
            }
        }
        else if (choice == 2) {
            if (Login(sock)) {
                loggedIn = true;
            }
        }
        else {
            printf("Invalid choice.\n");
        }

        // If logged in, proceed to chat panel
        if (loggedIn) {
            while (true) {
                // Send data
                printf("Enter text to send to the server (Type 'exit' to Logout): ");
                char sendData[1024];
                fgets(sendData, sizeof(sendData), stdin);

                // Remove newline character from the input
                size_t len = strlen(sendData);
                if (sendData[len - 1] == '\n') {
                    sendData[len - 1] = '\0';
                }

                // Check for "exit" keyword to logout
                if (strcmp(sendData, "exit") == 0) {
                    loggedIn = false;
                    break;
                }

                std::string encryptedData = encryptCaesarCipher(sendData, 5);
                std::string messageWithUsername = username + ": " + encryptedData;
                if (send(sock, messageWithUsername.c_str(), messageWithUsername.size(), 0) == SOCKET_ERROR) {
                    printf("send failed: %d\n", WSAGetLastError());
                    closesocket(sock);
                    WSACleanup();
                    return 1;
                }
                else {
                    printf("Data sent to server \n");
                }
            }
        }
    } 

    // Close the socket
    closesocket(sock);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}