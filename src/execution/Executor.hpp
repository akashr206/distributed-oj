#pragma once 
#include <string> 

class Executor {
    std::string container_name;
public:
    Executor(const std::string& container_name);
    void execute(const std::string& id, 
                        const std::string& code, 
                        const std::string& input, 
                        const std::string& language = "cpp");
};