#include "tcp_server.h"

TCPServer::TCPServer(int port, int max_clients, Tintin_reporter & logger) : max_clients(max_clients), logger(logger)
{
    lock_switch();
    logger.info("Creating TCP server");
    create_socket(port, max_clients);
    logger.info((std::string("TCP server listening on port ") + std::to_string(port)).c_str());
}

TCPServer::~TCPServer()
{
    std::cout << "Graceful stop!" << std::endl;
    close(sock);
    lock_switch();
    logger.info("TCP server stopped");
}

void TCPServer::create_socket(int port, int max_clients)
{
    struct sockaddr_in addr;
    
    sock = socket(PF_INET, SOCK_STREAM, 0);
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(sock, max_clients);
}

void TCPServer::serve_forever()
{
    fd_set sockets;
    int max;

    while (run)
    {
        FD_ZERO(&sockets);
        FD_SET(sock, &sockets);
        for (auto s = client_sockets.begin(); s != client_sockets.end(); s++)
            FD_SET(*s, &sockets);

        auto m = std::max_element(client_sockets.begin(), client_sockets.end());
        max = std::max(sock, (m == client_sockets.end() ? 0 : *m));

        if (select(max + 1, &sockets, NULL, NULL, NULL) < 0)
        {
            if (errno == EINTR)
                return;
            logger.error("Fatal: internal server error (select) !");
            return;
        }

        for (auto s = client_sockets.begin(); s != client_sockets.end(); s++)
            if (FD_ISSET(*s, &sockets))
                if (!get_message(*s))
                    if ((s = client_sockets.erase(s)) == client_sockets.end())
                        break;
        if (FD_ISSET(sock, &sockets))
            handle_new_conn();
    }
}

int TCPServer::get_message(int client_sock)
{
    int length;

    bzero(buf, sizeof(buf));
    if ((length = recv(client_sock, buf, sizeof(buf), 0)) <= 0)
    {
        close(client_sock);
        logger.info((std::string("Client ") + std::to_string(client_sock) + std::string(" disconnected!")).c_str());
        return 0;
    }
    if (buf[length - 1] == '\n')
        buf[length - 1] = 0;
    if (!handle_msg(buf))
        logger.msg(buf, client_sock);
    return 1;
}

bool TCPServer::handle_msg(char *msg)
{
    if (!strcmp(msg, "quit"))
    {
        run = false;
        logger.info("Server shutdown requested");
        return true;
    }
    return false;
}

void TCPServer::send_msg(int sock, int code)
{
    auto str = std::to_string(code);
    send(sock, str.c_str(), str.length(), 0);
}

void TCPServer::handle_new_conn()
{
    int client_sock = accept(sock, NULL, NULL);
    if (client_sockets.size() == (unsigned int)max_clients)
    {
        logger.warning("Deny new connection, server too busy");
        send_msg(client_sock, codes::TOO_BUSY);
        close(client_sock);
    }
    else
    {
        logger.info((std::string("New connection ! Client socket : ") + std::to_string(client_sock)).c_str());
        send_msg(client_sock, codes::OK);
        client_sockets.insert(client_sock);
    }
}

void TCPServer::signal_handler(int sig)
{
    logger.warning((std::string("Signal intercepted: ") + std::string(strsignal(sig))).c_str());
    run = false;
}

bool TCPServer::is_running()
{
    return std::filesystem::exists(std::filesystem::path("/var/lock/matt_daemon.lock"));
}

void TCPServer::lock_switch()
{
    if (TCPServer::is_running())
        remove("/var/lock/matt_daemon.lock");
    else
        std::ofstream("/var/lock/matt_daemon.lock");
}
