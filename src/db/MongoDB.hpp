#pragma once

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

class MongoDB
{
private:
    mongocxx::instance instance_;
    mongocxx::client client_;
public:
    static MongoDB& getInstance() {
        static MongoDB instance;
        return instance;
    }
    MongoDB();
    mongocxx::database getDB();
};

