#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <fstream> // For file operations
#include <string>
#include <thread> // For std::thread

#pragma comment(lib, "Ws2_32.lib")

SOCKET clientSock;
std::string username;
// Get the handle to the console (colors)
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

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
void ReceiveMessages(SOCKET clientSock) {

    while (true) {

        // Receive message from client
        char recvmsg[1024];
        int msgSize = recv(clientSock, recvmsg, sizeof(recvmsg), 0);
        if (msgSize == SOCKET_ERROR) {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
            printf("Client disconnected\n", WSAGetLastError());
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            closesocket(clientSock);
            return;
        }
        else if (msgSize == 0) {
            // Connection closed by client
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
            printf("Client disconnected\n");
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            closesocket(clientSock);
            return;
        }

        recvmsg[msgSize] = '\0';
        std::string decryptedData = caesarDecrypt(recvmsg, 5);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
        printf("Received message from %s\n", decryptedData.c_str(), "\n");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        

    }
}

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

    // Password rules validation
    std::string passString(password);
    if (passString.length() < 8) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        printf("Password must be at least 8 characters long.\n");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        return false;
    }

    bool hasNumber = false;
    bool hasSpecial = false;
    for (char c : passString) {
        if (isdigit(c)) {
            hasNumber = true;
        }
        else if (!isalpha(c) && !isdigit(c)) {
            hasSpecial = true;
        }
    }

    if (!hasNumber) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        printf("Password must include at least one number.\n");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        return false;
    }

    if (!hasSpecial) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        printf("Password must include at least one special character.\n");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        return false;
    }

    // Open the user file to check for duplicates
    std::ifstream userFile("server_user_info.txt");
    std::string line;
    while (std::getline(userFile, line)) {
        std::string EncDuplicate = encryptCaesarCipher(tempUsername, 5);
        if (line.find(EncDuplicate) != std::string::npos) {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
            printf("Username already exists. Please choose another one.\n");
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            return false;
        }
    }
    userFile.close();


    // Save the username and password to the file
    std::ofstream outFile("server_user_info.txt", std::ios::app);
    if (outFile.is_open()) {
        std::string encryptedPass = encryptCaesarCipher(password, 5);
        std::string encryptedUser = encryptCaesarCipher(tempUsername, 5);
        outFile << encryptedUser << " " << encryptedPass << std::endl;
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

    // Check if username or password is empty
    if (strlen(tempUsername) == 0 || strlen(password) == 0) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        printf("Login failed. Username or password cannot be empty.\n");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        return false;
    }

    // Open the user file to check for matching credentials
    std::ifstream userFile("server_user_info.txt");
    std::string line;
    bool found = false;
    while (std::getline(userFile, line)) {
        std::string encryptedPassw = encryptCaesarCipher(password, 5);
        std::string encryptedUsern = encryptCaesarCipher(tempUsername, 5);
        if (line.find(encryptedUsern) != std::string::npos && line.find(encryptedPassw) != std::string::npos) {
            // Check if the entire username and password match
            size_t passIndex = line.find(' ');
            std::string storedUsern = line.substr(0, passIndex);
            std::string storedPassw = line.substr(passIndex + 1);
            if (encryptedUsern == storedUsern && encryptedPassw == storedPassw) {
                found = true;
                break;
            }
        }
    }
    userFile.close();

    // If matching credentials found, return true; otherwise, return false
    if (found) {
        username = tempUsername;
        printf("Login successful.\n");
        return true;
    }
    else {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        printf("Login failed. Invalid username or password.\n");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        return false;
    }
}

int main() {
    
    std::cout << R"(

       _____  _             _                              _    _____  _  _               _     __ 
      / ____|| |           | |       /\                   | |  / ____|| |(_)             | |   /_ |
     | |     | |__    __ _ | |_     /  \    _ __   _ __   | | | |     | | _   ___  _ __  | |_   | |
     | |     | '_ \  / _` || __|   / /\ \  | '_ \ | '_ \  | | | |     | || | / _ \| '_ \ | __|  | |
     | |____ | | | || (_| || |_   / ____ \ | |_) || |_) | | | | |____ | || ||  __/| | | || |_   | |
      \_____||_| |_| \__,_| \__| /_/    \_\| .__/ | .__/  | |  \_____||_||_| \___||_| |_| \__|  |_|
                                           | |    | |     | |                                      
                                           |_|    |_|     |_|                                      

    )" << std::endl;

    // Get the handle to the console (colors)
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    bool loggedIn = false;
    while (!loggedIn) {
        // Ask the client to sign up or login
        printf("1. Sign up\n2. Login\nChoose an option (Ctrl + C to end app at anytime): ");
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

        printf("-----Server started-----\n");
        std::cout << "--Wait for Client to LogIn then Enter message in blank space to send to client ('exit' to end client)-- \n";


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
        std::thread(ReceiveMessages, clientSock).detach();

        // Main loop to continuously send messages to the client
        while (true) {
            // Send message to client
            std::string message;
            std::getline(std::cin, message);
            std::string messageUsername = username + ": " + message;
            std::string encryptedData = encryptCaesarCipher(messageUsername, 5);
            if (send(clientSock, encryptedData.c_str(), encryptedData.size(), 0) == SOCKET_ERROR) {
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
                printf("Client Not Available :(\n");
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                closesocket(clientSock);
                break;
            }
            else {
                SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
                printf("Message sent to client\n");
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            }

            if (message == "exit") { // Compare the string directly
                std::string disconnect = ": Client Logged Out ";
                std::string logoutenc = encryptCaesarCipher(disconnect, 5);
                if (send(clientSock, logoutenc.c_str(), logoutenc.size(), 0) == SOCKET_ERROR) {
                    printf("send failed: %d\n", WSAGetLastError());
                    return false;
                }
                loggedIn = false;
                break;
            }
        }
    }



    // Cleanup Winsock
    WSACleanup();

    return 0;
}
