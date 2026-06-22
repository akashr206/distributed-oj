#include "execution/QueueListener.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include "execution/Executor.hpp"
#include "models/Jobs.hpp"

QueueListener::QueueListener(const std::string& redisUrl, const std::string& qName, const std::string& cName) : redis(redisUrl), queueName(qName), container_name(cName) {
    std::cout<< "Connected to Redis Queue: " << qName << std:: endl;
} 

void QueueListener::listen() {
    std::cout << "Listening to the queue : " << queueName << std::endl;

    while(true){
        auto res = redis.brpop(queueName, 0);

        if(res){
            auto subId = res->second;
            std::cout << "[WORKER] Picked up new task" << std:: endl;
            std::cout << "[Payload] " << subId << std::endl;
            
            auto job = JobRepo::getInstance().getById(subId);
            std::cout << job << std::endl;
            Executor executor(container_name);
            executor.execute(subId, job.code, job.input);
            

            // call executor and extract output
            // call evaluator to evaluate the code
            // save result to db
        }
    }

}