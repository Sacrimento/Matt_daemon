#include "conf.h"

const char *get_config_file_path()
{
    char *path = std::getenv("MD_CONFIG");
    if (!path && !std::filesystem::is_regular_file("./matt_daemon.conf"))
        throw ParserException("Could not find config file");
    return (path ? path : "./matt_daemon.conf");
}

bool is_number(const std::string& s)
{
    auto it = s.begin();
    
    if (*it == '-')
        it++;

    return !s.empty() && std::find_if(it, 
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

std::string get_from_conf(std::map<std::string, std::string> &conf, const char *key, const char *_default, bool use_default)
{
    auto it = conf.find(key);

    if (it == conf.end())
    {
        if (use_default)
            return _default;
        return std::string("");
    }
    return it->second;
}

std::string get_path_from_conf(std::map<std::string, std::string> &conf, const char *key, const char *_default, bool use_default)
{
    std::filesystem::path path = get_from_conf(conf, key, _default, use_default);
    if (std::filesystem::is_directory(path) || !std::filesystem::is_directory(path.parent_path()))
        throw ParserException(std::string(path) + ": invalid path");
    return path;
}

int get_int_from_conf(std::map<std::string, std::string> &conf, const char *key, int _default, bool use_default)
{
    int ret;
    std::string s = get_from_conf(conf, key, std::to_string(_default).c_str(), use_default);
    if (!is_number(s))
        throw ParserException(std::string("invalid value for ") + key + std::string(" (excepted int type)"));
    try {
        if (s.empty())
            ret = -1;
        else
            ret = std::stoi(s);
    } catch (std::invalid_argument &e) {
        throw ParserException(std::string("invalid value for ") + key + std::string(" (excepted int type)"));
    } catch (std::out_of_range &e) {
        throw ParserException(std::string("invalid value for ") + key + std::string(" (excepted int type)"));
    }
    if (ret < 0)
        throw ParserException(std::string("invalid value for ") + key + std::string(" (excepted positive int type)"));
    return ret;
}

std::map<std::string, std::string> parse_config_file(const char *path)
{
    std::ifstream file(path);
    std::string line, section, name, value;
    long unsigned int delim, line_i = 0;
    std::map<std::string, std::string> map;

    if (!file.is_open() || !file.good())
        throw ParserException(std::string("could not open ") + std::string(path));

    while (std::getline(file, line))
    {
        line_i++;
        line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
        line.erase(std::remove_if(line.begin(), line.end(), [](char c){ return c == '"'; }), line.end());
        line.erase(std::remove_if(line.begin(), line.end(), [](char c){ return c == '\''; }), line.end());
        if (line[0] == '#' || line.empty())
            continue;

        if (line[0] == '[')
        {
            if ((delim = line.find(']')) == std::string::npos)
                throw ParserException(std::string("line") + std::string(":") + std::to_string(line_i) + std::string(" : invalid section name"));
            section = line.substr(1, delim - 1);
            continue;
        }

        if ((delim = line.find('=')) == std::string::npos)
            throw ParserException(std::string("line") + std::string(":") + std::to_string(line_i) + std::string(" : missing '='"));
        name = line.substr(0, delim);
        value = line.substr(delim + 1);

        if (section.empty())
            throw ParserException(std::string("line") + std::string(":") + std::to_string(line_i) + std::string(" : missing section before assignment"));
        if (!name.empty() && !value.empty())
            map.insert(std::pair<std::string, std::string>(section + "/" + name, value));
        else
            throw ParserException(std::string("line") + std::string(":") + std::to_string(line_i) + std::string(" : bad expression"));
    }
    return map;
}

ParserException::ParserException(std::string err) throw() : _err(err) {}
const char *ParserException::what() const throw() { return this->_err.c_str(); }
