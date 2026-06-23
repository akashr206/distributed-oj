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
    
    while(true){
        try{
            std::cout << "Listening to the queue : " << queueName << std::endl;
            auto res = redis.brpop(queueName, 0);

            if(res){
                auto subId = res->second;
                auto jobRepo = JobRepo();
                Job job;
                try{
                    job = jobRepo.getById(subId);
                    std::cout << "[WORKER] Picked up new task" << std:: endl;
                    std::cout << "[Payload] " << subId << std::endl;

                    jobRepo.updateStatus(subId, "running");

                    Executor executor;
                    executor.execute(subId, job);

                    jobRepo.updateStatus(subId, "completed");

                    std::cout << job << std::endl;
                }
                catch(const std::exception& e){
                    std::cerr << "[JOB ERROR]" <<e.what() << '\n';
                    job.internalError = e.what();
                    jobRepo.updateStatus(subId, "failed");
                }
                catch(...){
                    std::cerr << "[JOB ERROR] Unknown critical error" << std::endl;
                    job.internalError = "Unknown critical error";
                    jobRepo.updateStatus(subId, "failed");
                }
            }
        }
        catch (const sw::redis::Error& e) {
            std::cerr << "[REDIS ERROR] " << e.what() << "\n";
            std::cerr << "Retrying in 5 seconds\n";
            std::this_thread::sleep_for(std::chrono::seconds(5));
        } 
        catch (const std::exception& e) {
            std::cerr << "[SYSTEM ERROR] " << e.what() << "\n";
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
}