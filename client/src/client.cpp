#include "client.h"

bool handle_err(const char *buf)
{
    int code = std::stoi(buf);
    switch (code)
    {
        case 1:
            std::cerr << "Could not connect : server too busy" << std::endl;
            break;
        case 2:
            std::cerr << "Could not connect : authentification failed" << std::endl;
            break;
        default:
            return false;
    }
    return true;
}

bool _recv(int sock, unsigned long *pub, unsigned long *prd)
{
    char buf[4096];
    std::string line, part;
    int delim = 0;

    if (recv(sock, buf, sizeof(buf), 0) < 0)
    {
        printf("Could not receive response from server\n");
        return false;
    }

    std::istringstream sbuf(buf);
    std::getline(sbuf, line);

    if (handle_err(line.c_str()))
        return false;

    if (pub && prd)
    {
        std::getline(sbuf, line);
        delim = line.find(':');
        part = line.substr(0, delim);
        std::cout << part << std::endl;
        *pub = std::stoul(part);
        part = line.substr(delim + 1);
        *prd = std::stoul(part);
    }
    return true;
}

bool _send(int sock, std::string msg, unsigned long srv_pub, unsigned long srv_prd)
{
    if (srv_pub && srv_prd)
        msg = RSAEncryption::encrypt_msg(msg, srv_pub, srv_prd);
    return !(send(sock, msg.c_str(), msg.length(), 0) == -1);
}

int connect(int port)
{
    int sock = 0;
    struct sockaddr_in addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
   
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
       
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
   
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    return sock;
}

void sig_handler(int sig)
{
    (void)sig;
    g_run = false;
    fclose(stdin);
}

void setup_signals()
{
    signal(SIGINT, sig_handler);
}

void run(int sock, unsigned long srv_pub, unsigned long srv_prd)
{
    std::string line;
    char b;

    std::cout << "===============================================\n=                   Ben AFK                   =\n===============================================" << std::endl;
    while (g_run)
    {
        if (recv(sock, &b, 1, MSG_PEEK | MSG_DONTWAIT) == 0)
        {
            std::cerr << "Connection closed" << std::endl; 
            break;
        }
        std::cout << "Ben_AFK> ";
        if (!std::getline(std::cin, line))
            break;
        if (line.length() > 4095)
        {
            std::cerr << std::endl << "Error: can not send more than 4095 bytes" << std::endl;
            continue;
        }
        if (!_send(sock, line, srv_pub, srv_prd))
        {
            if (errno == EBADF)
                std::cerr << "TCP Server unreachable" << std::endl;
            else
                std::cerr << errno << std::endl;
            break;
        }
        if (line == "quit")
            break;
    }
}

int main(void)
{
    int sock;
    unsigned long srv_rsa_pub, srv_rsa_prd;
    auto [rsa_pub, rsa_priv, rsa_prd] = RSAEncryption::gen_key_pairs(0xFFFF);

    if ((sock = connect(4242)) == -1)
        return 1;
    setup_signals();

    if (_recv(sock, &srv_rsa_pub, &srv_rsa_prd) && _send(sock, std::string("rsa\n") + std::to_string(rsa_pub) + ":" + std::to_string(rsa_prd), 0, 0))
        run(sock, srv_rsa_pub, srv_rsa_prd);

    close(sock);

    return 0;
}