#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <csignal>
#include <map>
#include <algorithm>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "logger.h"
#include "conf.h"
#include "encryption.h"

class TCPServer
{
    private:
        struct client {
            unsigned long rsa_public_key;
            unsigned long rsa_prd;
            std::string name;
        };
        
        int max_clients;
        int sock;
        bool run = true;
        char buf[4096];
        unsigned long rsa_private_key;
        unsigned long rsa_public_key;
        unsigned long rsa_prd;
        std::map<int, client> clients;
        std::map<std::string, std::string> auth;
        Tintin_reporter & logger;

        void lock_switch();
        std::pair<std::string, std::string> two_args(std::string arg);
        void parse_auth_file(std::string auth_file);
        bool auth_client(int cl_sock, client &c, std::string creds);
        void create_socket(int port, int max_clients);
        bool get_message(int cl_sock, client &c);
        void send_msg(int sock, std::string msg, int code, unsigned long key, unsigned long prd);
        void handle_new_conn();
        bool handle_msg(std::istringstream msg, int cl_sock, client &c);
        bool disconnect_client(int cl_sock, std::string reason);

    public:
        enum codes
        {
            OK,
            TOO_BUSY,
            AUTH_FAIL,
            AUTH_ALREADY
        };

        TCPServer(int port, int max_clients, std::string auth_path, Tintin_reporter & logger);
        ~TCPServer();
        static bool is_running();
        void serve_forever();
        void signal_handler(int sig);
        void reload_conf();
};

extern TCPServer *g_tcp_server;
