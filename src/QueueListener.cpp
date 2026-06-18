#include "QueueListener.hpp"
#include <iostream>
#include <thread>
#include <chrono>

QueueListener::QueueListener(const std::string& redisUrl, const std::string& qName) : redis(redisUrl), queueName(qName) {
    std::cout<< "Connected to Redis Queue: " << qName << std:: endl;
} 

void QueueListener::listen() {
    std::cout << "Listening to the queue : " << queueName << std::endl;

    while(true){
        auto res = redis.brpop(queueName, 0);

        if(res){
            auto payload = res->second;
            std::cout << "[WORKER] Picked up new task" << std:: endl;
            std::cout << "[Payload] " << payload << std::endl;
        }
    }

}