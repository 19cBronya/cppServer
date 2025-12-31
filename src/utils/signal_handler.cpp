#include "utils/signal_handler.h"
#include <csignal>
#include <cstdlib>

SignalHandler& SignalHandler::getInstance() {
    static SignalHandler instance;
    return instance;
}

SignalHandler::SignalHandler() : m_shouldExit(false) {
}

SignalHandler::~SignalHandler() {
}

void SignalHandler::registerHandler(int signal, std::function<void()> handler) {
    if (signal == SIGINT) {
        m_sigintHandler = handler;
    } else if (signal == SIGTERM) {
        m_sigtermHandler = handler;
    }
}

void SignalHandler::setup() {
    std::signal(SIGINT, SignalHandler::handleSignal);
    std::signal(SIGTERM, SignalHandler::handleSignal);
}

void SignalHandler::handleSignal(int sig) {
    SignalHandler& instance = getInstance();
    
    if (sig == SIGINT || sig == SIGTERM) {
        instance.m_shouldExit = true;
        if (sig == SIGINT && instance.m_sigintHandler) {
            instance.m_sigintHandler();
        } else if (sig == SIGTERM && instance.m_sigtermHandler) {
            instance.m_sigtermHandler();
        }
    }
}

