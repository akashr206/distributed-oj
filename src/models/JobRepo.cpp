#include "models/Jobs.hpp"
#include "db/MongoDB.hpp"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <algorithm>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;


std::string getString(const bsoncxx::document::view& view,
                      const std::string& key) {
    auto elem = view[key];
    if (elem && elem.type() == bsoncxx::type::k_string)
        return std::string(elem.get_string().value);

    return "";
}

Job Job::fromBson(bsoncxx::document::view view) {
    Job job;

    job.code = getString(view, "code");
    job.input = getString(view, "input");
    job.expected = getString(view, "expected");
    job.language = getString(view, "language");
    job.output = getString(view, "output");
    job.verdict = getString(view, "verdict");
    job.status = getString(view, "status");

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