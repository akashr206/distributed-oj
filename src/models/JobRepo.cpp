#include "models/Jobs.hpp"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/json.hpp>
#include <algorithm>
#include <chrono>
#include <vector>

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


JobRepo::JobRepo(){
    db = MongoDB::getInstance().getDB();
    collection = db["jobs"];
}

Job JobRepo::getById(std::string id){
    auto res = collection.find_one(make_document(kvp("_id", bsoncxx::oid{id})));
    
    if(!res){
        std::cout << "No result found " << std::endl;
        return Job{};
    }
    return Job::fromBson(res->view());
}

void JobRepo::updateStatus(std::string id, std::string status){
    auto filter = make_document(kvp("_id", bsoncxx::oid{id}));
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();

    auto data = make_document(kvp("$set", make_document(kvp("status", status), kvp("updatedAt", ms))));
    
    auto res = collection.update_one(filter.view(), data.view());
    if(res){
        std::cout << "Matched: " << res->matched_count() << std::endl;
    } else {
        std::cout << "Job not found." << std::endl;
    }
}


void JobRepo::updateJob(std::string id, Job& job){
    auto filter = make_document(kvp("_id", bsoncxx::oid{id}));
    auto update = make_document(
        kvp("$set", make_document(
            kvp("status", job.status),
            kvp("output", job.output),
            kvp("verdict", job.verdict),

            kvp("timeTaken", job.timeTaken),
            kvp("memoryUsed", job.memoryUsed),

            kvp("error", job.error),
            kvp("internalError", job.internalError)
        ))
    );

    auto result = collection.update_one(filter.view(), update.view());

    if (!result) {
        std::cerr << "Failed to update job " << id << std::endl;
    }
}

void JobRepo::cleanDeadJob(){
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
    auto diffe = now_ms - (30 * 1000);

    auto filter = make_document(
        kvp("status", "running"),
        kvp("updatedAt", make_document(kvp("$lte", diffe)))
    );

    auto update = make_document(kvp("$set", make_document(
        kvp("status", "failed"),
        kvp("internalError", "Worker crashed; please resubmit")
    )));

    mongocxx::options::find options{};
    options.projection(make_document(kvp("_id", 1)));
    options.batch_size(5000);

    auto cursor = collection.find(filter.view(), options);

    mongocxx::options::bulk_write bulk_opts;
    bulk_opts.ordered(false);

    std::vector<mongocxx::model::write> batch_ops;
    constexpr int batch_size = 5000;

    for (auto&& doc : cursor) {
        auto id = doc["_id"].get_value();
        auto match = make_document(kvp("_id", id));

        mongocxx::model::update_one update_op(match.view(), update.view());
        batch_ops.push_back(update_op);

        if (batch_ops.size() >= batch_size) {
            auto bulk = collection.create_bulk_write(bulk_opts);
            for (const auto& op : batch_ops) {
                bulk.append(op);
            }
            bulk.execute();
            batch_ops.clear();
        }
    }

    if (!batch_ops.empty()) {
        auto bulk = collection.create_bulk_write(bulk_opts);
        for (const auto& op : batch_ops) {
            bulk.append(op);
        }
        bulk.execute();
    }
}