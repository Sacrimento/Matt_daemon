#pragma once

#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <filesystem>

#include <stdio.h>

class Tintin_reporter
{
    public:
        enum level {
            DEBUG,
            INFO,
            WARNING,
            ERROR,
            EXCEPTION
        };

        static constexpr const char *level_str[5] = {
            "DEBUG",
            "INFO",
            "WARNING",
            "ERROR",
            "EXCEPTION"
        };
    
    private:

        std::string base_path = "";
        std::string name = "Tintin_reporter";
        std::string format = "[ %s ] %s | %s: %s";
        level _level = level::DEBUG;
        std::filesystem::path path;
        std::filesystem::path archive;
        std::ofstream file;
        int incr = 0;
        int max_lines = 0;
        int lines = 0;


        void init_files();
        void output(const char *l, const char *msg);
        void check_file();

    public:
        Tintin_reporter(std::string path);
        Tintin_reporter(std::string path, level l);
        Tintin_reporter(std::string path, std::string name);
        Tintin_reporter(std::string path, std::string name, level l);
        // Tintin_reporter(const)
        ~Tintin_reporter();

        static Tintin_reporter::level level_from_str(std::string str_l);
        static bool valid_level_str(std::string str_l);

        void set_path(std::filesystem::path _path);
        void set_level(level l) { _level = l; }
        void set_name(std::string n) { name = n; }
        void set_max_lines_per_file(int l) { max_lines = l; }

        void log(level l, const char *msg) { if (l >= _level) output(level_str[l], msg); }
        void log(const char *custom_level, const char *msg) { output(custom_level, msg); }

        void debug(const char *msg) { log(level::DEBUG, msg); }
        void info(const char *msg) { log(level::INFO, msg); }
        void msg(const char *msg, int sock) { log((std::string("USER (") + std::to_string(sock) + std::string(")")).c_str(), msg); }
        void warning(const char *msg) { log(level::WARNING, msg); }
        void error(const char *msg) { log(level::ERROR, msg); }
        void exception(const char *msg) { log(level::EXCEPTION, msg); }
};
