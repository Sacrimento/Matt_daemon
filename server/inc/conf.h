#pragma once

#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <cctype>
#include <cstdlib>
#include <map>

const char *get_config_file_path();
std::map<std::string, std::string> parse_config_file(const char *path);
std::string get_from_conf(std::map<std::string, std::string> &conf, const char *key, const char *_default, bool use_default);
std::string get_path_from_conf(std::map<std::string, std::string> &conf, const char *key, const char *_default, bool must_exist, bool use_default);
int get_int_from_conf(std::map<std::string, std::string> &conf, const char *key, int _default, bool use_default);

class ParserException : public std::exception
{
    private:
        std::string _err;

    public:
        ParserException(std::string err) throw();
        virtual const char  *what() const throw();
};
