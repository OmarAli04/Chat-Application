#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <fstream> // For file operations
#include <string>
#include <thread> // For std::thread

#pragma comment(lib, "Ws2_32.lib")

// Global variable username
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
    std::ifstream userFile("user_info.txt");
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
        std::string encryptedUser = encryptCaesarCipher(tempUsername, 5);
        outFile << encryptedUser << " " << encryptedPass << std::endl;
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

    // Check if username or password is empty
    if (strlen(tempUsername) == 0 || strlen(password) == 0) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        printf("Login failed. Username or password cannot be empty.\n");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        return false;
    }

    // Open the user file to check for matching credentials
    std::ifstream userFile("user_info.txt");
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
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        printf("Login failed. Invalid username or password.\n");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        return false;
        
    }


}

void ReceiveMessages(SOCKET sock) {
    while (true) {
        // Receive message from server
        char recvmsg[1024];
        int msgSize = recv(sock, recvmsg, sizeof(recvmsg), 0);
        if (msgSize == SOCKET_ERROR) {
            // Set text color to red
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
            printf("Client Disconnected\n");
            // Reset text color to default
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            closesocket(sock);
            break;
        }
        else if (msgSize == 0) {
            printf("Server disconnected\n");
            closesocket(sock);
            break;
        }
        recvmsg[msgSize] = '\0';
        // Decrypt received message using Caesar cipher decryption
        std::string decryptedData = caesarDecrypt(recvmsg, 5);
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
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        printf("Client 1 Offline");
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
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
        printf("1. Sign up\n2. Login\nChoose an option (Ctrl + C to end app at anytime): ");
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
                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
                    printf("Client 1 Offline");
                    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
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