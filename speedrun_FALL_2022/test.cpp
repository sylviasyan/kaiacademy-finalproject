#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/types.h>

std::string execute(const std::string& cmd_args)
{
    std::stringstream out;
    std::stringstream err;
    FILE* pipe = popen(cmd_args.c_str(), "r");
    if (!pipe)
    {
        std::cerr << "Error: popen failed" << std::endl;
        return "";
    }
    char buffer[128];
    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != NULL)
        {
            out << buffer;
        }
    }
    int status = pclose(pipe);
    if (status == -1)
    {
        std::cerr << "Error: pclose failed" << std::endl;
    }
    else if (WIFEXITED(status))
    {
        if (WEXITSTATUS(status) != 0)
        {
            err << "Error: command exited with status " << WEXITSTATUS(status) << std::endl;
        }
    }
    else if (WIFSIGNALED(status))
    {
        err << "Error: command terminated by signal " << WTERMSIG(status) << std::endl;
    }
    std::cout << out.str() << err.str();
    return (out.str() + err.str());
}

std::string sit_aware(std::vector<std::string> args)
{
    std::string username = execute("whoami");
    std::string hostname = execute("hostname");
    std::string os_type = "";
    std::string os_name = "";
    std::string os_version = "";
#if defined(_WIN32)
    os_type = "Windows";
    os_name = "Windows";
    os_version = "";
#elif defined(__APPLE__)
    os_type = "Darwin";
    os_name = "Mac OS X";
    os_version = "";
#else
    os_type = "Linux";
    os_name = "Linux";
    os_version = "";
#endif
    std::stringstream initial_data;
    initial_data << "username: " << username << std::endl;
    initial_data << "hostname: " << hostname << std::endl;
    initial_data << "os_type: " << os_type << std::endl;
    initial_data << "os_name: " << os_name << std::endl;
    initial_data << "os_version: " << os_version << std::endl;
    return initial_data.str();
}

int main()
{
    std::vector<std::string> args;
    std::cout << sit_aware(args) << std::endl;
    return