#include "execution/Executor.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

Executor::Executor(const std::string& cont_name) : container_name(cont_name) {}

void Executor::execute(const std::string& id, const std::string &code, const std::string &input, const std::string &language){

    std::filesystem::path rel_path = std::filesystem::path("../temp") / id;
    std::filesystem::create_directories(rel_path);

    std::string folder = std::filesystem::absolute(rel_path).string();

    std::ofstream wcode((folder + "/code.cpp").c_str());
    std::ofstream winput((folder + "/input.txt").c_str());
    if (!wcode.is_open()) {
        throw std::runtime_error("Critical Error: Failed to open code.cpp for writing!");
    }
    if (!winput.is_open()) {
        throw std::runtime_error("Critical Error: Failed to open input.cpp for writing!");
    }
    wcode << code  << std::endl;
    winput << input << std::endl;

    wcode.close();
    winput.close();

    std::string cmd =
    "docker run --rm "
    "--network none "
    "--cpus=4 "
    "--memory=2048m "
    "--pids-limit=20 "
    "--user=1000:1000 "
    "-w /workspace "
    "-v \"" + folder + "\":/workspace "
    "gcc:14 "
    "sh -c \""
    "g++ code.cpp -std=c++17 -o program && "
    "timeout 2s ./program < input.txt > output.txt"
    "\"";

    std::cout << cmd << std::endl;
    system(cmd.c_str());
    
    std::cout << "Execution for " << id << "Completed !" << std::endl;   
}