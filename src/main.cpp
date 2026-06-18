#include <iostream>
#include <string> 
#include "QueueListener.hpp"

int main() {

    try {
        std::string redis_url = "tcp://127.0.0.1:6379";
        std::string qName = "submissions_queue";
        QueueListener queueListener(redis_url, qName);
        queueListener.listen();

    } catch (const std::exception& e){
        std::cerr << "[ERROR] Something Went Wrong: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}