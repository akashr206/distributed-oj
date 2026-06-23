#include "execution/Executor.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <chrono>


std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Executor::execute(const std::string& id, Job& job){
    std::string code = job.code;
    std::string input = job.input;
    std::string language = job.language.empty() ? "cpp" : job.language;
    int timeLimit = job.timeLimit == 0 ? 2000 : job.timeLimit;
    int memoryLimit = job.memoryLimit == 0 ? 256 : job.memoryLimit;
    int timeInS = timeLimit / 1000;

    if(timeInS > 5) {
        throw std::runtime_error("Critical Error: timeLimit is beyond the max limit");
    }

    if(memoryLimit > 1024){
        throw std::runtime_error("Critical Error: memoryLimit is beyond the max limit");
    }


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

    std::string volumeMount = folder + ":/workspace";
    std::string memoryStr = std::to_string(memoryLimit) + "m";
    std::string timeLimitStr = std::to_string(timeInS);

    std::string shellCmd =
        "if ! g++ code.cpp -std=c++17 -o program 2> err.txt; then exit 201; fi; "
        "ulimit -f 1024; "
        "/usr/bin/time -f \"%M %e\" -o stats.txt "
        "timeout " + timeLimitStr + "s ./program < input.txt > output.txt 2> err.txt";

    std::vector<std::string> argStrs = {
        "docker", "run", "--rm",
        "--network", "none",
        "--cpus", "1",
        "--memory", memoryStr,
        "--pids-limit", "20",
        "--user", "1000:1000",
        "-w", "/workspace",
        "-v", volumeMount,
        "oj-cpp",
        "sh", "-c", shellCmd
    };

    std::vector<const char*> args;
    for (const auto& s : argStrs) {
        args.push_back(s.c_str());
    }
    args.push_back(nullptr);

    std::cout << "Executing code in sandbox" << std::endl;

    pid_t pid = fork();
    if (pid == -1) {
        throw std::runtime_error("fork() failed");
    }

    int systemStatus = 0;
    if (pid == 0) {
        execv("/usr/bin/docker", (char* const*)args.data());
        std::cerr << "execv() failed: " << strerror(errno) << std::endl;
        exit(127);
    } else {
        int status;
        waitpid(pid, &status, 0);
        systemStatus = status;
    }

    int memoryUsed = 0;
    double timeTaken = 0;

    std::ifstream stats(folder + "/stats.txt");
    if(stats){
        stats >> memoryUsed >> timeTaken;
    }
    job.memoryUsed = memoryUsed / 1024;
    job.timeTaken = static_cast<int> (timeTaken * 1000);

    int statusCode = WEXITSTATUS(systemStatus);

    std::string verdict = "";
    std::string error = "";
    std::string output;
    switch(statusCode){
        case 137:
            verdict = "MLE";
            break;
        case 201:
            verdict = "CE";
            error = readFile(folder + "/err.txt");
            break;
        case 124:
            verdict = "TLE";
            break;
        case 153:
            verdict = "OLE";
            break;
        case 125:
        case 126:
        case 127:
            verdict = "IE";
            job.status = "error";
            job.internalError = "Sandbox execution failed";
            break;
        case 0:
            output = readFile(folder + "/output.txt");
            if(output == job.expected) verdict = "AC";
            else verdict = "WA";
            break;

        default:
            verdict = "RE";
            error = readFile(folder + "/err.txt");
    }
    job.verdict = verdict;
    job.error = error;
    job.output = output.size() > 4096 ? output.substr(0, 4093) : output;
    std::filesystem::remove_all(folder);
    std::cout << "Execution for " << id << " Completed!" << std::endl;   
}