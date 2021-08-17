#include <unistd.h>
#include <sys/stat.h>

#include "tcp_server.h"
#include "logger.h"
#include "conf.h"

TCPServer *g_tcp_server = NULL;

void fail(std::string err)
{
    std::cerr << err << std::endl;
    std::cerr << "matt_daemon exiting..." << std::endl;
    exit(EXIT_FAILURE);
}

void daemonize_process()
{
    auto pid = fork();

    if (pid < 0)
        fail("Daemon initialization fail");
    if (pid > 0)
        exit(EXIT_SUCCESS);
    setsid();

    pid = fork();
    if (pid < 0)
        fail("Daemon initialization fail");
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
    signal(SIGHUP, sig_handler);
}

int main(void)
{
    if (getuid() != 0)
        fail("Fatal: you must be root to run the daemon");
    if (TCPServer::is_running())
        fail("Fatal: can not start daemon, an instance is already running");

    std::string l_path, l_name, l_level;
    int l_max_lines, s_port, s_max_clients;
    std::map<std::string, std::string> conf;

    try {
        conf = parse_config_file(get_config_file_path());
        l_path = get_path_from_conf(conf, "logger/path", "/var/log/matt_daemon.log", true);
        l_name = get_from_conf(conf, "logger/name", "matt_daemon", true);
        l_level = get_from_conf(conf, "logger/level", "INFO", true);
        l_max_lines = get_int_from_conf(conf, "logger/max_file_lines", 3, true);
        s_port = get_int_from_conf(conf, "server/port", 4242, true);
        s_max_clients = get_int_from_conf(conf, "server/max_clients", 3, true);
    } catch (ParserException &e) {
        fail(std::string("Config error: ") + e.what());
    }

    if (!Tintin_reporter::valid_level_str(l_level))
        fail(std::string("Config error: ") + l_level + ": invalid logger level");

    //daemonize_process();
    auto logger = Tintin_reporter(l_path, l_name, Tintin_reporter::level_from_str(l_level));
    logger.set_max_lines_per_file(l_max_lines);

    logger.info("Daemon started");
    auto s = TCPServer(s_port, s_max_clients, logger);

    g_tcp_server = &s;
    setup_signals();
    s.serve_forever();
    logger.info("Daemon exiting...");

    return EXIT_SUCCESS;
}