#include "tcp_server.h"

TCPServer::TCPServer(int port, int max_clients, std::string auth_path, Tintin_reporter & logger) : max_clients(max_clients), logger(logger)
{
    lock_switch();
    parse_auth_file(auth_path);
    logger.info("Creating TCP server");
    logger.info("Generating RSA key pairs...");
    auto[pub_key, priv_key, prd] = RSAEncryption::gen_key_pairs(0xFFFF);
    rsa_public_key = pub_key;
    rsa_private_key = priv_key;
    rsa_prd = prd;
    logger.debug((std::string("Public key: ") + RSAEncryption::get_str_key(rsa_public_key, rsa_prd)).c_str());
    create_socket(port, max_clients);
    logger.info((std::string("TCP server listening on port ") + std::to_string(port)).c_str());

}

TCPServer::~TCPServer()
{
    close(sock);
    for (auto c : clients)
        close(c.first);
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
        max = 0;
        for (auto c = clients.cbegin(); c != clients.cend(); c++)
        {
            FD_SET(c->first, &sockets);
            max = std::max(c->first, max);
        }
        max = std::max(sock, max);

        if (select(max + 1, &sockets, NULL, NULL, NULL) < 0)
        {
            if (errno != EINTR)
                logger.error("Fatal: internal server error (select) !");
            return;
        }

        for (auto &c: clients)
        {
            if (FD_ISSET(c.first, &sockets))
                if (!get_message(c.first, c.second))
                    break;
        }
        if (FD_ISSET(sock, &sockets))
            handle_new_conn();
    }
}

bool TCPServer::get_message(int cl_sock, client &c)
{
    int length;
    std::string s;

    bzero(buf, sizeof(buf));
    if ((length = recv(cl_sock, buf, sizeof(buf), 0)) <= 0)
        return disconnect_client(cl_sock, "");
    if (buf[length - 1] == '\n')
        buf[length - 1] = 0;
    s = buf;
    if (c.rsa_public_key && c.rsa_prd)
        s = RSAEncryption::decrypt_msg(s, rsa_private_key, rsa_prd);
    return handle_msg(std::istringstream(s), cl_sock, c);
}

bool TCPServer::handle_msg(std::istringstream s, int cl_sock, client &c)
{
    std::string line, part;

    std::getline(s, line);

    if (!c.rsa_public_key && !c.rsa_prd && line != "rsa")
        return disconnect_client(cl_sock, "Secure connection was not established");

    if (line == "quit")
    {
        run = false;
        logger.info("Server shutdown requested");
    }
    else if (line == "reload")
        reload_conf();
    else if (line == "rsa")
    {
        std::getline(s, line);
        if (!RSAEncryption::store_to_int(line, &c.rsa_public_key, &c.rsa_prd))
            return disconnect_client(cl_sock, "Invalid RSA key exchange");
        logger.debug((std::string("Received ") + c.name + " public key: " + RSAEncryption::get_str_key(c.rsa_public_key, c.rsa_prd)).c_str());
    }
    else if (line == "auth")
    {
        std::getline(s, line);
        return auth_client(cl_sock, c, line);
    }
    else
        logger.msg(line.c_str(), c.name);
    return true;
}

void TCPServer::send_msg(int sock, std::string msg, int code, unsigned long pub_key, unsigned long prd)
{
    auto str = std::to_string(code) + "\n" + msg;

    if (pub_key && prd)
        str = RSAEncryption::encrypt_msg(str, pub_key, prd);

    if (send(sock, str.c_str(), str.length(), 0) == -1)
        logger.exception("Could not send msg");
}

void TCPServer::handle_new_conn()
{
    client c = {0, 0, ""};

    int cl_sock = accept(sock, NULL, NULL);
    c.name = std::string("Guest") + std::to_string(cl_sock);
    if (clients.size() == (unsigned int)max_clients)
    {
        logger.warning("Deny new connection, server too busy");
        send_msg(cl_sock, "", codes::TOO_BUSY, 0, 0);
        close(cl_sock);
    }
    else
    {
        logger.info((std::string("New connection ! Client socket : ") + std::to_string(cl_sock)).c_str());
        send_msg(cl_sock, std::to_string(rsa_public_key) + ":" + std::to_string(rsa_prd), codes::OK, 0, 0);
        logger.debug("Sent server public key");
        clients.insert(std::pair<int, client>(cl_sock, c));
    }
}

bool TCPServer::auth_client(int cl_sock, client &c, std::string creds)
{
    std::pair<std::string, std::string> p = two_args(creds);

    if (p.first.empty() || p.second.empty())
    {
        send_msg(cl_sock, "", codes::AUTH_FAIL, c.rsa_public_key, c.rsa_prd);
        return disconnect_client(cl_sock, "Invalid authentification arguments");
    }

    if (auth.find(p.first) == auth.end() || auth[p.first] != p.second)
    {
        send_msg(cl_sock, "", codes::AUTH_FAIL, c.rsa_public_key, c.rsa_prd);
        return disconnect_client(cl_sock, "Invalid login/password");
    }

    for (auto &cl : clients)
        if (cl.second.name == p.first)
        {
            send_msg(cl_sock, "", codes::AUTH_ALREADY, c.rsa_public_key, c.rsa_prd);
            return disconnect_client(cl_sock, p.first + " is already logged in");
        }
    c.name = p.first;
    return true;
}

bool TCPServer::disconnect_client(int cl_sock, std::string reason)
{
    std::string msg = "Client " + clients[cl_sock].name + " disconnected. " + reason;
    logger.info(msg.c_str());
    close(cl_sock);

    for (auto c = clients.begin(); c != clients.end(); c++)
        if (c->first == cl_sock)
            return !(clients.erase(c) == clients.end());
    return true;
}

void TCPServer::signal_handler(int sig)
{
    if (sig == SIGHUP)
        reload_conf();
    logger.warning((std::string("Signal intercepted: ") + std::string(strsignal(sig))).c_str());
    run = false;
}

void TCPServer::reload_conf()
{
    std::string l_path, l_name, l_level, s_auth_path;
    int l_max_lines, s_max_clients;
    std::map<std::string, std::string> conf;

    logger.info("Config reload requested");
    try {
        conf = parse_config_file(get_config_file_path());
        l_path = get_path_from_conf(conf, "logger/path", "/var/log/matt_daemon.log", false, false);
        l_name = get_from_conf(conf, "logger/name", "matt_daemon", false);
        l_level = get_from_conf(conf, "logger/level", "INFO", false);
        l_max_lines = get_int_from_conf(conf, "logger/max_file_lines", 3, false);
        s_max_clients = get_int_from_conf(conf, "server/max_clients", 3, false);
        s_auth_path = get_path_from_conf(conf, "server/auth_file", "./matt_daemon_secret", true, false);
    } catch (ParserException &e) {
        logger.exception(e.what());
        logger.warning("Aborting config reload");
        return;
    }

    if (!Tintin_reporter::valid_level_str(l_level))
    {
        logger.exception((std::string("Config error: ") + l_level + ": invalid logger level").c_str());
        logger.warning("Aborting config reload");
        return;
    }

    if (!l_path.empty())
        logger.set_path(l_path);
    if (!l_name.empty())
        logger.set_name(l_name);
    if (!l_level.empty())
        logger.set_level(Tintin_reporter::level_from_str(l_level));
    if (l_max_lines != -1)
        logger.set_max_lines_per_file(l_max_lines);
    if (s_max_clients != -1)
        max_clients = s_max_clients;
    if (!s_auth_path.empty())
        parse_auth_file(s_auth_path);

    logger.info("Config reloaded !");
}

void TCPServer::parse_auth_file(std::string auth_path)
{
    std::ifstream file(auth_path);
    std::string line;
    std::pair<std::string, std::string> p;

    if (!file.is_open() || !file.good())
        return;

    while (std::getline(file, line))
    {
        if (line.empty())
            continue;
        p = two_args(line);
        if (p.first.empty() || p.second.empty())
        {
            logger.error((std::string("Auth file error: \"") + line + "\". Ignoring line").c_str());
            continue;
        }
        if (auth.find(p.first) != auth.end())
        {
            logger.error((std::string("Auth file error: ") + p.first + "\" already exists. Ignoring line").c_str());
            continue;
        }
        auth.insert(p);
    }

    file.close();
}

std::pair<std::string, std::string> TCPServer::two_args(std::string arg)
{
    size_t delim;

    if ((delim = arg.find(':')) == std::string::npos)
        return std::pair<std::string, std::string>("", "");
    
    return std::pair<std::string, std::string>(arg.substr(0, delim), arg.substr(delim + 1));
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
