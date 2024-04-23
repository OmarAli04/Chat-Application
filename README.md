# Chat Application using Winsock Socket Programming

## Server Application Overview
The communication system's core is the server-side implementation. It provides a safe sign-up and log-in process and creates the messaging platform for the second client. The server accepts connections from clients by using Winsock socket programming to listen for incoming connections on a designated port. It then spawns specialized threads to manage communication with each client. The server protects data integrity and confidentiality by using encryption and user authentication.
## Client Application Overview
The application gives users an easy-to-use interface on the client side through which they can communicate with the server (another client). Users can use Caesar cipher encryption to securely transmit sensitive data when creating new accounts or logging in with their existing login credentials. Upon successful authentication, clients can communicate with other users by sending messages through to the server, which enhances the smooth and safe communication process. 

## Features
- Sign up: Users can sign up with a unique username and password.
- Login: Registered users can log in to their accounts.
- Real-time messaging: Users can send and receive messages instantly.
- Encryption: Messages are encrypted using the Caesar cipher for security.

## Prerequisites
- Windows operating system
- Visual Studio (for compiling C++ code)
- Basic knowledge of C++ programming
- Understanding of socket programming concepts

## Installation
1. Clone or download the repository to your local machine.
2. Open the project in Visual Studio.
3. Compile and build the project.
4. Run the server application on one machine and the client application on another machine.
5. Sign up or log in with a username and password.
6. Start sending and receiving messages.

## Usage
- Server:
  - Run the server application.
  - Sign up or log in with a username and password.
  - Wait for clients to connect.
- Client:
  - Run the client application.
  - Sign up or log in with a username and password.
  - Start sending and receiving messages.

