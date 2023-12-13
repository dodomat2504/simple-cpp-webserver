#include "h/http/server.h"
#include <stdexcept>
#include "h/logger.h"


int HTTPServer::maxConnections = 10;

HTTPServer::HTTPServer() {
    root = new Endpoint("/");
}

void HTTPServer::addRoute(const std::string& route, const HTTP_METHOD method, std::function<http::Response(const http::Request&)> callback) {
    const std::vector<std::string> splitRoute = Endpoint::split(route);
    const int n = splitRoute.size();

    if (route == "/" && n == 0) {
        if (root->hasCallbackFor(method))
            throw std::runtime_error("Route '" + route + "' (" + HTTP_METHOD_toString(method) + ") already exists");
        else
            root->addCallback(method, callback);
    } else {
        Endpoint* current = root;

        for (int i = 0; i < n; i++) {
            const std::string& routePart = splitRoute[i];

            if (i == n - 1) {
                // last route part
                if (current->hasChildRoute(routePart)) {
                    if ((*current)[routePart]->hasCallbackFor(method))
                        throw std::runtime_error("Route '" + route + "' (" + HTTP_METHOD_toString(method) + ") already exists");
                    else
                        (*current)[routePart]->addCallback(method, callback);

                } else {
                    Endpoint* child = new Endpoint(routePart, (*current).fullPath());
                    child->addCallback(method, callback);
                    current->addChild(child);
                }
            } else {
                // not last route part
                if (current->hasChildRoute(routePart)) {
                    // route already exists
                    current = (*current)[routePart];
                } else {
                    // route does not exist
                    Endpoint* child = new Endpoint(routePart, (*current).fullPath());
                    current->addChild(child);
                    current = child;
                }
            }
        }
    }
}

void HTTPServer::GET(const std::string& route, std::function<http::Response(const http::Request&)> callback) {
    addRoute(route, HTTP_METHOD::GET, callback);
}

void HTTPServer::POST(const std::string& route, std::function<http::Response(const http::Request&)> callback) {
    addRoute(route, HTTP_METHOD::POST, callback);
}

void HTTPServer::PUT(const std::string& route, std::function<http::Response(const http::Request&)> callback) {
    addRoute(route, HTTP_METHOD::PUT, callback);
}

void HTTPServer::DELETE(const std::string& route, std::function<http::Response(const http::Request&)> callback) {
    addRoute(route, HTTP_METHOD::DELETE, callback);
}

std::thread* HTTPServer::start(const int port, std::atomic_bool* running) {
    // start listener
    std::thread* listen = new std::thread([this, port, running]() {
        tcp::openListener(port, HTTPServer::maxConnections, tcpConnectionRequestHandler, this, *running);
    });
    
    return listen;
}

void HTTPServer::stop() {
    std::lock_guard<std::mutex> lock(tcpConnections_mutex);

    // close all connections
    for (TCP_CONN_INFO* info : this->tcpConnections) {
        info->stop();
        delete info;
    }

    delete this->root;
}

HTTPServer::~HTTPServer() {
    stop();
}

void HTTPServer::tcpConnectionRequestHandler(const sockaddr_in address, HTTPServer* s, const int socketid) {

    Logger::debug("Verbindungsversuch von " + std::string(inet_ntoa(address.sin_addr)) + ":" + std::to_string(ntohs(address.sin_port)));

    {
        std::lock_guard<std::mutex> lock(s->tcpConnections_mutex);
        for (auto it = s->tcpConnections.begin(); it != s->tcpConnections.end(); ++it) {
            TCP_CONN_INFO* info = *it;

            if (! info->Running()) {
                delete info;
                s->tcpConnections.erase(it);

                if (s->tcpConnections.size() == 0)
                    break;
                else
                    it = s->tcpConnections.begin();
            }
        }
    }

    // check if max connections is reached
    if (s->tcpConnections.size() >= HTTPServer::maxConnections) {
        Logger::out("Maximale Anzahl an Verbindungen erreicht");
        return;
    }

    Logger::debug("Verbindungsaufbau mit " + std::string(inet_ntoa(address.sin_addr)) + ":" + std::to_string(ntohs(address.sin_port)));

    {
        std::lock_guard<std::mutex> lock(s->tcpConnections_mutex);
        // the Bank creates a new thread for each connection and stores it
        TCP_CONN_INFO* info = new TCP_CONN_INFO(address, new std::atomic_bool(true), socketid, nullptr);
        std::thread* t = new std::thread([s, info]() { s->HTTPConnectionHandler(info); });
        info->setThread(t);
        s->tcpConnections.push_back(info);
    }
}

http::Response HTTPServer::processHTTPRequest(const http::Request& req) const {
    const std::vector<std::string> splitRoute = Endpoint::split(req.header.Path);
    const int n = splitRoute.size();

    if (req.header.Path == "/" && n == 0) {
        if (root->hasCallbackFor(req.header.Method))
            return root->getCallback(req.header.Method)(req);
    } else {
        const Endpoint* current = root;

        for (int i = 0; i < n; i++) {
            const std::string& routePart = splitRoute[i];

            if (i == n - 1) {
                // last route part

                if (current->hasChildRoute(routePart) && (*current)[routePart]->hasCallbackFor(req.header.Method))
                    return (*current)[routePart]->getCallback(req.header.Method)(req);
                else
                    break;

            } else {
                // not last route part

                if (current->hasChildRoute(routePart))
                    current = (*current)[routePart];
                else
                    break;
            }
        }
    }
    
    // If no route was found, return 404

    http::Response res;

    res.header.StatusCode = 404;
    res.header.StatusMessage = "Not Found";
    res.header.ContentType = CONTENT_TYPE::TEXT;
    res.header.Version = "HTTP/1.1";
    res.header.Connection = "close";

    res.body.data = "Route '" + req.header.Path + "' (" + HTTP_METHOD_toString(req.header.Method) + ") not found\r\n\r\n";

    return res;
}

void HTTPServer::HTTPConnectionHandler(TCP_CONN_INFO* info) {
    Logger::debug("Verbindung mit " + std::string(inet_ntoa(info->Address().sin_addr)) + ":" + std::to_string(ntohs(info->Address().sin_port)) + " hergestellt");

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100 * 1000;

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(info->Socket(), &read_fds);

    const int select_ret = select(info->Socket() + 1, &read_fds, NULL, NULL, &tv);

    if (select_ret == -1) {
        Logger::err("Error occurred while waiting for acknowledgment packet");
        exit(EXIT_FAILURE);
    } else if (select_ret == 0) {
        // timeout
        Logger::debug("Timeout");
    } else {
        const int bufferSize = 2048;
        char buffer[bufferSize] = {0};

        std::string data = "";

        int n = 0;

        while ((n = tcp::rcv(info->Socket(), buffer, bufferSize) ) > 0) {
            if (n == -1)
                throw std::runtime_error("Error");
            
            if (n == 0)
                break;

            data += buffer;

            // detect end of request
            if(buffer[n-1] == '\n')
                break;
            
            // detect end of request if no \n is found
            if(n!=bufferSize) 
                break;

            for (int i = 0; i < bufferSize; i++)
                buffer[i] = 0;
        }

        if (data.size() > 0) {
            const http::Request req = http::parseHTTPRequest(data);
            const http::Response res = this->processHTTPRequest(req);

            const std::string response = http::serializeHTTPResponse(res);
            tcp::send(response, info->Socket())->join();
        }
    }

    info->stop();
    close(info->Socket());

    Logger::debug("Verbindung mit " + std::string(inet_ntoa(info->Address().sin_addr)) + ":" + std::to_string(ntohs(info->Address().sin_port)) + " geschlossen");
}

void HTTPServer::printRoutes() const {
    Logger::out("Routes:");
    root->printAllRoutes();
}
