#pragma once
#include <string>
#include <sw/redis++/redis++.h>

class QueueListener {
private:
    sw::redis::Redis redis;
    std::string queueName;
    std::string container_name;
    public:
    QueueListener(const std::string& redisUrl, const std::string& qName, const std::string& cName);

    void listen();
};