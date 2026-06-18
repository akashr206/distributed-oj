#pragma once
#include <string>
#include <sw/redis++/redis++.h>

class QueueListener {
private:
    sw::redis::Redis redis;
    std::string queueName;
public:
    QueueListener(const std::string& redisUrl, const std::string& qName);

    void listen();
};