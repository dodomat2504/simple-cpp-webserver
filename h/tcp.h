#pragma once

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <atomic>
#include <thread>

#include "server.h"


namespace tcp {

    
    /**
     * @brief opens a listener for new connections on the given port
     * @param port the port to listen on
     * @param maxConnections the maximum amount of connections
     * @param messageHandler the function to call when a message is received
     * @param obj the object to pass to the messageHandler
     * @param running the atomic bool to check if the listener should still run
    */
    void openListener(const int port, const int maxConnections, void (*messageHandler)(const sockaddr_in, HTTPServer*, const int), HTTPServer* obj, std::atomic_bool& running);

    /**
     * @brief sends a message to the given address
     * @param msg the message to send
     * @param n the length of the message
     * @param address the address to send the message to
     * @param socket the socket to send the message with
    */
    std::thread* send(const std::string msg, const int socket);

    /**
     * @brief receives a message from the given socket
     * @param socket the socket to receive the message from
     * @param buffer the buffer to store the message in
     * @param n the length of the buffer
     * @return the amount of bytes received
    */
    int rcv(const int socket, char* buffer, const int n);

}
