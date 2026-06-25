#include <iostream>
#include <string> 
#include <vector>
#include <thread>
#include "execution/QueueListener.hpp"
#include "models/Jobs.hpp"


void spinThread(int threadNum){
    std::string containerName = "worker_tread_" + std::to_string(threadNum);
    std::cout << "Initializing thread number: " << threadNum << std::endl;
    
    std::string redisUrl = "tcp://127.0.0.1:6379";
    std::string qName = "submissions_queue";
    try
    {
        QueueListener queueListener(redisUrl, qName, containerName);
        queueListener.listen();
    }
    catch(const std::exception& e)
    {
        std::cerr << containerName << " crashed: " << e.what() << '\n';
    }
}

void spinZombieJobKiller(){
    try{
        JobRepo jobRepo;
        while(true){
            jobRepo.cleanDeadJob();
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    } catch(const std::exception& e)
    {
        std::cerr << "Zombie killer crashed: " << e.what() << '\n';
    }
}

int main() {

    // try {
    //     std::string redis_url = "tcp://127.0.0.1:6379";
    //     std::string qName = "submissions_queue";
    //     std::string container_name = "gcc_worker_" + std::to_string(rand() % 100000);
    //     QueueListener queueListener(redis_url, qName, container_name);
    //     queueListener.listen();

    // } catch (const std::exception& e){
    //     std::cerr << "[ERROR] Something Went Wrong: " << e.what() << std::endl;
    //     return 1;
    // }

    unsigned int numWorkers = std::thread::hardware_concurrency();

    if(numWorkers > 8) numWorkers = 8;
    
    std::cout << "Initializing " << numWorkers << " worker threads.\n";
    std::vector<std::thread> threads;

    for(unsigned int i = 0; i < numWorkers - 1; ++i){
        threads.emplace_back(spinThread, i);
    }

    threads.emplace_back(spinZombieJobKiller);

    for(auto& t : threads){
        if(t.joinable()) t.join();
    }

    return 0;
}