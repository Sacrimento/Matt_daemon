#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <csignal>
#include <unordered_set>
#include <algorithm>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "logger.h"

class TCPServer
{
    private:
        int max_clients;
        int sock;
        bool run = true;
        char buf[4096];
        std::unordered_set<int> client_sockets;
        Tintin_reporter & logger;

        void lock_switch();
        void create_socket(int port, int max_clients);
        int get_message(int client_sock);
        void send_msg(int sock, int code);
        void handle_new_conn();
        bool handle_msg(char *buf);

    public:
        enum codes
        {
            OK,
            TOO_BUSY,
            AUTH_FAIL
        };

        TCPServer(int port, int max_clients, Tintin_reporter & logger);
        ~TCPServer();
        static bool is_running();
        void serve_forever();
        void signal_handler(int sig);
};

extern TCPServer *g_tcp_server;
