#include <unistd.h>
#include <sys/stat.h>

#include "tcp_server.h"
#include "logger.h"
#include "conf.h"

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

    std::string l_path, l_name, l_level;
    int l_max_lines, s_port, s_max_clients;
    std::map<std::string, std::string> conf;

    try {
        conf = parse_config_file(get_config_file_path());
        l_path = get_from_conf(conf, "logger/path", "/var/log/matt_daemon.log", true);
        l_name = get_from_conf(conf, "logger/name", "matt_daemon", true);
        l_level = get_from_conf(conf, "logger/level", "INFO", true);
        l_max_lines = get_int_from_conf(conf, "logger/max_file_lines", 3, true);
        s_port = get_int_from_conf(conf, "server/port", 4242, true);
        s_max_clients = get_int_from_conf(conf, "server/max_clients", 3, true);
    } catch (ParserException &e) {
        std::cerr << "Config error: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    auto logger = Tintin_reporter(l_path, l_name, Tintin_reporter::level_from_str(l_level));
    logger.set_max_lines_per_file(l_max_lines);

        //daemonize_process();
    logger.info("Daemon started");
    auto s = TCPServer(s_port, s_max_clients, logger);

    g_tcp_server = &s;
    setup_signals();
    s.serve_forever();
    logger.info("Daemon exiting...");

    return EXIT_SUCCESS;
}