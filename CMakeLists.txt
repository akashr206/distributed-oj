cmake_minimum_required(VERSION 3.10)
project(DistributedOnlineJudge)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
add_executable(judge_worker src/main.cpp src/execution/QueueListener.cpp src/execution/Executor.cpp)

target_include_directories(judge_worker PRIVATE src/)
target_link_libraries(judge_worker PRIVATE Threads::Threads redis++ hiredis)