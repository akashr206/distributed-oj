#include <iostream>
#include <string> 
#include <algorithm>
#include "execution/QueueListener.hpp"


int main() {

    try {
        std::string redis_url = "tcp://127.0.0.1:6379";
        std::string qName = "submissions_queue";
        std::string container_name = "gcc_worker_" + std::to_string(rand() % 100000);
        QueueListener queueListener(redis_url, qName, container_name);
        queueListener.listen();

    } catch (const std::exception& e){
        std::cerr << "[ERROR] Something Went Wrong: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}