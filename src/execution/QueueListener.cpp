#include "execution/QueueListener.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include "execution/Executor.hpp"

using json = nlohmann::json;

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
            
            std::ifstream pifs("../src/models/problems.json");
            std::ifstream sifs("../src/models/submissions.json");

            json pjf = json::parse(pifs);
            json sjf = json::parse(sifs);

            int subInd = -1;
            for(int i = 0; i < sjf.size(); i++){
                if(sjf[i]["id"] == subId){
                    subInd = i;
                    break;
                }
            }
            int pInd = -1;
            for(int i = 0; i < pjf.size(); i++){
                if(pjf[i]["id"] == sjf[i]["problem_id"]){
                    pInd = i;
                    break;
                }
            }
            Executor executor(container_name);
            executor.execute(subId, sjf[subInd]["code"], pjf[pInd]["test_cases"]);
            // call executor and extract output
            // call evaluator to evaluate the code
            // save result to db
        }
    }

}