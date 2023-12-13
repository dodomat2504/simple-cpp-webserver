#pragma once

#include <netinet/in.h>
#include <atomic>
#include <thread>
#include <exception>


class TCP_CONN_INFO {
private:
    const sockaddr_in address;
    std::atomic_bool* running;
    const int socket;
    std::thread* thread;
public:
    TCP_CONN_INFO(const sockaddr_in address, std::atomic_bool* running, const int socket, std::thread* thread):
        address(address), running(running), socket(socket), thread(thread) {}

    ~TCP_CONN_INFO() {
        *running = false;
        thread->join();

        delete running;
        delete thread;
    }

    sockaddr_in Address() const { return address; }
    bool Running() const { return *running; }
    int Socket() const { return socket; }
    std::thread* Thread() const { return thread; }

    void setThread(std::thread* thread) { if (this->thread == nullptr) this->thread = thread; else throw std::runtime_error("tcp_conn_info: setThread"); }
    void stop() { (*running) = false; }
};
