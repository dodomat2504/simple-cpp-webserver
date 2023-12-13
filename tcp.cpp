#include "h/tcp.h"
#include <iostream>


void tcp::openListener(const int port, const int maxConnections, void (*messageHandler)(const sockaddr_in, HTTPServer*, const int), HTTPServer* obj, std::atomic_bool& running) {
    int serverFd;
    struct sockaddr_in address;
    const int opt = 1;
  
    // Creating socket file descriptor
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
  
    // Setting socket options
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
  
    // attaching socket to the specified port
    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, maxConnections) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (running) {
        int newSocket;
        struct sockaddr_in clientAddress;
        int addrLen = sizeof(clientAddress);
        
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 50 * 1000;

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(serverFd, &read_fds);

        const int select_ret = select(serverFd + 1, &read_fds, NULL, NULL, &tv);

        if (select_ret == -1) {
            exit(EXIT_FAILURE);
        } else if (select_ret == 0) {
            continue;
        } else {
            if ((newSocket = accept(serverFd, (struct sockaddr*)&clientAddress, (socklen_t*)&addrLen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            messageHandler(clientAddress, obj, newSocket);
        }
    }

    shutdown(serverFd, SHUT_RDWR);
}

std::thread* tcp::send(const std::string msg, const int socket) {
    return new std::thread([msg, socket]() {
        write(socket, msg.c_str(), msg.size());
    });
}

int tcp::rcv(const int socket, char* buffer, const int n) {
    return read(socket, buffer, n);
}
