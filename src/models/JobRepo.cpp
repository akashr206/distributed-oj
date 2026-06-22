#include "models/Jobs.hpp"
#include "db/MongoDB.hpp"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <algorithm>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

std::string getString(const bsoncxx::document::view& view, const std::string& key) {
    auto elem = view[key];
    if (elem && elem.type() == bsoncxx::type::k_string)
        return std::string(elem.get_string().value);
    return "";
}

int getInt(const bsoncxx::document::view& view, const std::string& key, int default_val = 0) {
    auto elem = view[key];
    if (elem) {
        if (elem.type() == bsoncxx::type::k_int32) return elem.get_int32().value;
        if (elem.type() == bsoncxx::type::k_int64) return static_cast<int>(elem.get_int64().value);
        if (elem.type() == bsoncxx::type::k_double) return static_cast<int>(elem.get_double().value);
    }
    return default_val;
}

double getDouble(const bsoncxx::document::view& view, const std::string& key, double default_val = 0.0) {
    auto elem = view[key];
    if (elem) {
        if (elem.type() == bsoncxx::type::k_double) return elem.get_double().value;
        if (elem.type() == bsoncxx::type::k_int32) return static_cast<double>(elem.get_int32().value);
        if (elem.type() == bsoncxx::type::k_int64) return static_cast<double>(elem.get_int64().value);
    }
    return default_val;
}

Job Job::fromBson(bsoncxx::document::view view) {
    Job job;

    job.code = getString(view, "code");
    job.input = getString(view, "input");
    job.expected = getString(view, "expected");
    job.language = getString(view, "language");
    job.status = getString(view, "status");
    job.output = getString(view, "output");
    job.verdict = getString(view, "verdict");
    job.error = getString(view, "error");
    job.internalError = getString(view, "internalError");

    job.timeLimit = getInt(view, "timeLimit");
    job.memoryLimit = getInt(view, "memoryLimit");
    job.timeTaken = getInt(view, "timeTaken");

    job.memoryUsed = getDouble(view, "memoryUsed");

    return job;
}

Job JobRepo::getById(std::string id){
    mongocxx::database db = MongoDB::getInstance().getDB();
    auto collection = db["jobs"];

    auto res = collection.find_one(make_document(kvp("_id", bsoncxx::oid{id})));
    
    if(!res){
        std::cout << "No result found " << std::endl;
        return Job{};
    }
    return Job::fromBson(res->view());
}