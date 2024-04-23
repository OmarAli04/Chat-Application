#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <fstream> // For file operations
#include <string>
#include <thread> // For std::thread

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

// Global variable username
std::string username;
// Get the handle to the console (colors)
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

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
    std::ifstream userFile("user_info.txt");
    std::string line;
    while (std::getline(userFile, line)) {
        if (line.find(tempUsername) != std::string::npos) {
            printf("Username already exists. Please choose another one.\n");
            return false;
        }
    }
    userFile.close();

    // Send the signup data to the server
    std::string signUpData = std::string(tempUsername) + ": Signed Up ";
    std::string EncsignData = encryptCaesarCipher(signUpData, 5);
    if (send(sock, EncsignData.c_str(), EncsignData.size(), 0) == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        return false;
    }

    // Save the username and password to the file
    std::ofstream outFile("user_info.txt", std::ios::app);
    if (outFile.is_open()) {
        std::string encryptedPass = encryptCaesarCipher(password, 5);
        outFile << tempUsername << " " << encryptedPass << std::endl;
        outFile.close();
        username = tempUsername;
        printf("Sign up successful.\n");
        printf("Enter messages in blank space to send to client (Type 'exit' to Logout): \n");
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
        // Send the login data to the server
        std::string loginData = std::string(tempUsername) + ": Logged In ";
        std::string EncloginData = encryptCaesarCipher(loginData, 5);
        if (send(sock, EncloginData.c_str(), EncloginData.size(), 0) == SOCKET_ERROR) {
            printf("send failed: %d\n", WSAGetLastError());
            return false;
        }
        printf("Login successful.\n");
        printf("Enter messages in blank space to send to client (Type 'exit' to Logout): \n");
        return true;
    }
    else {
        printf("Login failed. Invalid username or password.\n");
        return false;
        
    }


}

void ReceiveMessages(SOCKET sock) {
    while (true) {
        // Receive message from server
        char recvData[1024];
        int recvSize = recv(sock, recvData, sizeof(recvData), 0);
        if (recvSize == SOCKET_ERROR) {
            // Set text color to red
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
            printf("Client Disconnected\n");
            // Reset text color to default
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            closesocket(sock);
            break;
        }
        else if (recvSize == 0) {
            printf("Server disconnected\n");
            closesocket(sock);
            break;
        }
        recvData[recvSize] = '\0';
        // Decrypt received message using Caesar cipher decryption
        std::string decryptedData = caesarDecrypt(recvData, 5);
        // Set text color to orange
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
        printf("Received message from %s\n", decryptedData.c_str());
        // Reset text color to default
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
}

int main() {

    std::cout << R"(


       _____  _             _                              _    _____  _  _               _     ___  
      / ____|| |           | |       /\                   | |  / ____|| |(_)             | |   |__ \ 
     | |     | |__    __ _ | |_     /  \    _ __   _ __   | | | |     | | _   ___  _ __  | |_     ) |
     | |     | '_ \  / _` || __|   / /\ \  | '_ \ | '_ \  | | | |     | || | / _ \| '_ \ | __|   / / 
     | |____ | | | || (_| || |_   / ____ \ | |_) || |_) | | | | |____ | || ||  __/| | | || |_   / /_ 
      \_____||_| |_| \__,_| \__| /_/    \_\| .__/ | .__/  | |  \_____||_||_| \___||_| |_| \__| |____|
                                           | |    | |     | |                                        
                                           |_|    |_|     |_|                                        

    )" << std::endl;

    // Get the handle to the console (colors)
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

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

    std::thread(ReceiveMessages, sock).detach();
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
            std::thread(ReceiveMessages, sock).detach();
            while (true) {
                // Send data
                
                char sendData[1024];
                fgets(sendData, sizeof(sendData), stdin);

                // Remove newline character from the input
                size_t len = strlen(sendData);
                if (sendData[len - 1] == '\n') {
                    sendData[len - 1] = '\0';
                }

                // Check for "exit" keyword to logout
                if (strcmp(sendData, "exit") == 0) {
                    std::string disconnect = ": Client Logged Out ";
                    std::string logoutenc = encryptCaesarCipher(disconnect, 5);
                    if (send(sock, logoutenc.c_str(), logoutenc.size(), 0) == SOCKET_ERROR) {
                        printf("send failed: %d\n", WSAGetLastError());
                        return false;
                    }
                    loggedIn = false;
                    break;
                }
                // Send message to server
                std::string messageUsername = username + ": " + sendData;
                std::string encryptedData = encryptCaesarCipher(messageUsername, 5);
                //std::string messageWithUsername = username + ":" + sendData;
                if (send(sock, encryptedData.c_str(), encryptedData.size(), 0) == SOCKET_ERROR) {
                    printf("send failed: %d\n", WSAGetLastError());
                    closesocket(sock);
                    WSACleanup();
                    return 1;
                }
                else {
                    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
                    printf("Message sent to client\n");
                    // Reset text color to default
                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
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