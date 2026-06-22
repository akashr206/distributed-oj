#pragma once 
#include <string> 
#include "models/Jobs.hpp"

class Executor {
public:
    Executor() = default;
    void execute(const std::string& id, Job& job);
};