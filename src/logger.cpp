#include "logger.h"

Tintin_reporter::Tintin_reporter(std::string path) : base_path(path), path(path)
{
    init_files();
    check_file();
}

Tintin_reporter::Tintin_reporter(std::string path, level l) : base_path(path), _level(l), path(path)
{
    init_files();
    check_file();
}

Tintin_reporter::Tintin_reporter(std::string path, std::string name) : base_path(path), name(name), path(path)
{
    init_files();
    check_file();
}

Tintin_reporter::Tintin_reporter(std::string path, std::string name, level l) : base_path(path), name(name), _level(l), path(path)
{
    init_files();
    check_file();
}

void Tintin_reporter::init_files()
{
    time_t st;
    char buf[128];
    std::string tmp;

    st = time(NULL);
    file.open(path, std::ofstream::out | std::ofstream::app);
    strftime(buf, sizeof(buf), "%d%m%Y_%H%M%S", std::localtime(&st));
    tmp = std::string("/var/log/") + name + std::string("_archives/");
    std::filesystem::create_directory(tmp);
    if (std::filesystem::is_directory(tmp))
    {
        archive = std::string(tmp) + name + std::string(buf);
        std::filesystem::create_directory(archive);
    }
}

void Tintin_reporter::check_file()
{
    if (lines + 1 == max_lines)
        file.close();

    while ((!file.is_open() || !file.good() || !std::filesystem::exists(path)) && incr < 128)
    {
        if (std::filesystem::exists(path) && std::filesystem::is_directory(archive))
            std::filesystem::rename(path, archive / path.filename());
        path = std::filesystem::path(base_path + std::string(".") + std::to_string(++incr));
        if (file.is_open())
            file.close();
        file.open(path, std::ofstream::out | std::ofstream::app);
        lines = 0;
    }
}

void Tintin_reporter::output(const char *l, const char *msg)
{
    char *formated = NULL;
    char time_buf[128];

    check_file();
    time_t tm = time(NULL);
    std::strftime(time_buf, sizeof(time_buf), "%d/%m/%Y %H:%M:%S", std::localtime(&tm));
    asprintf(&formated, format.c_str(), name.c_str(), time_buf, l, msg);
    file << formated << std::endl;
    lines++;
    free(formated);
}

Tintin_reporter::~Tintin_reporter()
{
    if (file.is_open())
        file.close();
    if (std::filesystem::exists(path) && std::filesystem::is_directory(archive))
    {
        std::cout << path << '>' << archive << std::endl;
        std::filesystem::rename(path, archive / path.filename());
    }
}