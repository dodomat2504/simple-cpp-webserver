#pragma once


class HTTPServer;

#include <vector>
#include <thread>
#include <netinet/in.h>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <arpa/inet.h>
#include <iostream>
#include <variant>
#include <functional>
#include "tcp_conn_info.h"
#include "tcp.h"
#include "http.h"
#include "endpoint.h"


class HTTPServer {
private:

    /// @brief Holds all the tcp connections
    std::vector<TCP_CONN_INFO*> tcpConnections;

    /// @brief max number of connections
    static int maxConnections;
    
    /// @brief Mutex for the tcpConnections vector
    std::mutex tcpConnections_mutex;

    /// @brief The Endpoints
    Endpoint* root;

    /// @brief Add a callback function for a route
    /// @param route the route to add
    /// @param method the HTTP method used
    /// @param callback the callback function
    void addRoute(const std::string& route, const HTTP_METHOD method, std::function<http::Response(const http::Request&)> callback);

    /// @brief Callback function for the tcp listener for incoming connection requests
    /// @param address Requesting address
    /// @param b Bank object
    /// @param socketid socket id of the new connection
    static void tcpConnectionRequestHandler(const sockaddr_in address, HTTPServer* s, const int socketid);

    /// @brief Handles the started http connection
    /// @param info struct holding the connection information
    void HTTPConnectionHandler(TCP_CONN_INFO* info);

    /// @brief Processes the http request
    /// @param req incoming http request
    /// @return generated http response
    http::Response processHTTPRequest(const http::Request& req) const;

protected:
    HTTPServer();

    ~HTTPServer();

    /// @brief Start the http server
    /// @param port the port to listen on
    std::thread* start(const int port, std::atomic_bool* running);

    /// @brief Stop the server
    void stop();

public:

    /// @brief Add a callback function for a GET route
    /// @param route the route to add
    /// @param callback the callback function
    void GET(const std::string& route, std::function<http::Response(const http::Request&)> callback);

    /// @brief Add a callback function for a POST route
    /// @param route the route to add
    /// @param callback the callback function
    void POST(const std::string& route, std::function<http::Response(const http::Request&)> callback);

    /// @brief Add a callback function for a PUT route
    /// @param route the route to add
    /// @param callback the callback function
    void PUT(const std::string& route, std::function<http::Response(const http::Request&)> callback);

    /// @brief Add a callback function for a DELETE route
    /// @param route the route to add
    /// @param callback the callback function
    void DELETE(const std::string& route, std::function<http::Response(const http::Request&)> callback);

};
