#pragma once
#include <iostream>
#include <string>
#include <bsoncxx/document/view.hpp>
#include "db/MongoDB.hpp"

struct Job {
    std::string code;
    std::string input;
    std::string expected;
    std::string language;
    int timeLimit;       // in ms
    int memoryLimit;     // in MB

    std::string status;  // pending, running, error, completed
    std::string output;  // max 4kb
    std::string verdict; // TLE, AC, WA, MLE, ER, IE, OLE
    
    int timeTaken;       // in ms
    double memoryUsed;   // in MB

    std::string error;
    std::string internalError;

    static Job fromBson(bsoncxx::document::view view);
};

inline std::ostream& operator<<(std::ostream& os, const Job& job) {
    os << "Job {\n"
       << "  --- Metadata ---\n"
       << "  status: " << job.status << "\n"
       << "  verdict: " << job.verdict << "\n"
       << "  language: " << job.language << "\n"
       << "  \n  --- Limits & Metrics ---\n"
       << "  timeLimit: " << job.timeLimit << " ms\n"
       << "  memoryLimit: " << job.memoryLimit << " MB\n"
       << "  timeTaken: " << job.timeTaken << " ms\n"
       << "  memoryUsed: " << job.memoryUsed << " MB\n"
       << "  \n  --- Errors ---\n"
       << "  error:\n" << (job.error.empty() ? "None" : job.error) << "\n"
       << "  internalError:\n" << (job.internalError.empty() ? "None" : job.internalError) << "\n"
       << "  \n  --- Payloads ---\n"
       << "  code:\n" << (job.code.empty() ? "None" : job.code) << "\n"
       << "  input:\n" << (job.input.empty() ? "None" : job.input) << "\n"
       << "  expected:\n" << (job.expected.empty() ? "None" : job.expected) << "\n"
       << "  output:\n" << (job.output.empty() ? "None" : job.output) << "\n"
       << "}";
    return os;
}

class JobRepo {

private:
    mongocxx::database db;
    mongocxx::collection collection;

public:
    JobRepo();

    Job getById(std::string id);
    void updateStatus(std::string id, std::string status);
    void updateJob(std::string id, Job& job);
};