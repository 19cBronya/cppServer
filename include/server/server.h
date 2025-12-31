#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <cstdint>

class Server {
public:
    Server(uint16_t port);
    ~Server();
    
    bool start();
    void stop();
    void run();
    
    bool isRunning() const { return m_running; }

private:
    uint16_t m_port;
    int m_listenFd;
    bool m_running;
    
    bool createSocket();
    bool bindAndListen();
    void cleanup();
};

#endif // SERVER_H

