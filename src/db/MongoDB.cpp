#include "db/MongoDB.hpp"

MongoDB::MongoDB() : client_(mongocxx::uri{"mongodb://localhost:27017"}){}

mongocxx::database MongoDB::getDB(){
    return client_["dist_oj"];
}