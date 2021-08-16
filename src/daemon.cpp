#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "tcp_server.h"
#include "logger.h"

TCPServer *g_tcp_server = NULL;

void daemonize_process()
{
    auto pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    setsid();

    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);
    umask(0);
    chdir("/");

}

void sig_handler(int sig)
{
    if (g_tcp_server)
        g_tcp_server->signal_handler(sig);
}

void setup_signals()
{
    signal(SIGINT, sig_handler);
    signal(SIGQUIT, sig_handler);
	signal(SIGTERM, sig_handler);
}

int main(void)
{
    if (getuid() != 0)
        exit(EXIT_FAILURE);
    if (TCPServer::is_running())
        exit(EXIT_FAILURE);

    auto logger = Tintin_reporter(std::string("/var/log/matt_daemon.log"), std::string("Matt_daemon"), Tintin_reporter::level::INFO);

    //daemonize_process();
    logger.info("Daemon started");
    auto s = TCPServer(7777, 1, logger);
    g_tcp_server = &s;
    setup_signals();
    s.serve_forever();
    logger.info("Daemon exited");
    
    return EXIT_SUCCESS;
}