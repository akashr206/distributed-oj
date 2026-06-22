cmake_minimum_required(VERSION 3.10)
project(DistributedOnlineJudge)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)

add_executable(judge_worker src/main.cpp src/execution/QueueListener.cpp src/execution/Executor.cpp src/models/JobRepo.cpp src/db/MongoDB.cpp)

target_include_directories(judge_worker PRIVATE src/)
target_link_libraries(
    judge_worker 
    PRIVATE 
    Threads::Threads 
    redis++ 
    hiredis
    mongo::mongocxx_shared
    mongo::bsoncxx_shared
)