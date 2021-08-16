#include "client.h"

bool handle_err(char *buf)
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

int connect(int port)
{
    int sock = 0;
    struct sockaddr_in addr;
    char buf[16];

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

    if (recv(sock, buf, sizeof(buf), 0) < 0)
    {
        printf("Could not receive response from server\n");
        return -1;
    }

    if (handle_err(buf))
        return -1;

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

void run(int sock)
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
        if (send(sock, line.c_str(), line.length(), 0) == -1)
        {
            if (errno == EBADF)
                std::cerr << "TCP Server unreachable" << std::endl;
            else
                std::cerr << "Error while sending" << std::endl;
            break;
        }
        if (line == "quit")
            break;
    }
}

int main(void)
{
    int sock;
    if ((sock = connect(7777)) == -1)
        return 1;

    setup_signals();

    run(sock);

    close(sock);

    return 0;
}