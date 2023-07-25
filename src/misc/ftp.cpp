#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

const int BUFFER_SIZE = 1024;

std::string receiveResponse(int socket) {
    char buffer[BUFFER_SIZE];
    std::string response;
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytesRead = recv(socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead <= 0) {
            break;
        }
        response += buffer;
        if (response.find("\r\n") != std::string::npos) {
            break;
        }
    }
    return response;
}

bool sendCommand(int socket, const std::string& command) {
    if (send(socket, command.c_str(), command.size(), 0) < 0) {
        std::cerr << "Failed to send command: " << command << std::endl;
        return false;
    }
    return true;
}

bool connectToServer(const std::string& serverIP, int serverPort, int& socket) {
    socket = socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    if (inet_pton(AF_INET, serverIP.c_str(), &(serverAddress.sin_addr)) <= 0) {
        std::cerr << "Invalid server IP address" << std::endl;
        close(socket);
        return false;
    }

    if (connect(socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Failed to connect to the server" << std::endl;
        close(socket);
        return false;
    }

    return true;
}

bool login(int socket, const std::string& username, const std::string& password) {
    std::string command = "USER " + username + "\r\n";
    if (!sendCommand(socket, command)) {
        return false;
    }
    std::string response = receiveResponse(socket);
    std::cout << response;

    command = "PASS " + password + "\r\n";
    if (!sendCommand(socket, command)) {
        return false;
    }
    response = receiveResponse(socket);
    std::cout << response;

    return true;
}

bool enterPassiveMode(int socket, std::string& dataIP, int& dataPort) {
    std::string command = "PASV\r\n";
    if (!sendCommand(socket, command)) {
        return false;
    }
    std::string response = receiveResponse(socket);
    std::cout << response;

    // Parse the server's response to extract the IP address and port number for the data connection
    size_t openingParenthesisPos = response.find('(');
    size_t closingParenthesisPos = response.find(')', openingParenthesisPos);
    std::string addressData = response.substr(openingParenthesisPos + 1, closingParenthesisPos - openingParenthesisPos - 1);

    std::vector<std::string> addressParts;
    std::stringstream ss(addressData);
    std::string part;
    while (getline(ss, part, ',')) {
        addressParts.push_back(part);
    }

    if (addressParts.size() != 6) {
        std::cerr << "Failed to parse the server's response for passive mode" << std::endl;
        return false;
    }

    dataIP = addressParts[0] + "." + addressParts[1] + "." + addressParts[2] + "." + addressParts[3];
    dataPort = std::stoi(addressParts[4]) * 256 + std::stoi(addressParts[5]);

    return true;
}

int main() {
    std::string serverIP = "ftp.gnu.com";
    int serverPort = 21;
    std::string username = "scutech";
    std::string password = "dingjia";

    int controlSocket;
    if (!connectToServer(serverIP, serverPort, controlSocket)) {
        return 1;
    }

    std::string response = receiveResponse(controlSocket);
    std::cout << response;

    if (!login(controlSocket, username, password)) {
        close(controlSocket);
        return 1;
    }

    std::string dataIP;
    int dataPort;
    if (!enterPassiveMode(controlSocket, dataIP, dataPort)) {
        close(controlSocket);
        return 1;
    }

    // Now you can use the dataIP and dataPort to establish a data connection and perform FTP operations

    close(controlSocket);
    return 0;
}