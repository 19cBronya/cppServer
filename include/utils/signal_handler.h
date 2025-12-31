#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <functional>

class SignalHandler {
public:
    static SignalHandler& getInstance();
    
    void registerHandler(int signal, std::function<void()> handler);
    void setup();
    
    bool shouldExit() const { return m_shouldExit; }
    void setExit() { m_shouldExit = true; }

private:
    SignalHandler();
    ~SignalHandler();
    SignalHandler(const SignalHandler&) = delete;
    SignalHandler& operator=(const SignalHandler&) = delete;
    
    static void handleSignal(int sig);
    
    bool m_shouldExit;
    std::function<void()> m_sigintHandler;
    std::function<void()> m_sigtermHandler;
};

#endif // SIGNAL_HANDLER_H

