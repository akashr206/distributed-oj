# Distributed OJ

A simple distributed online judge system that accepts code submissions, stores them in MongoDB, queues them in Redis, and processes them with a worker binary.

## Overview

This project has three main parts:

- A Node.js server in the server directory that accepts submissions via HTTP.
- A C++ worker process that listens for queued jobs and executes submitted code.
- MongoDB and Redis as the backing services for job storage and queueing.

## Project Structure

- server/ - Express API server
- src/ - C++ worker implementation
- build/ - CMake build output
- docker/ - container-related files

## Prerequisites

Make sure these are installed:

- Node.js and npm
- CMake
- A C++ compiler with C++17 support
- MongoDB
- Redis

## Installation

### 1. Clone the repository

```bash
git clone <repository-url>
cd distributed_oj
```

### 2. Install Node.js dependencies

```bash
cd server
npm install
```

### 3. Build the worker

```bash
mkdir -p build
cd build
cmake ..
make
```

### 4. Start the required services

Start MongoDB and Redis locally or through Docker.

Example with Docker:

```bash
docker compose up -d
```

If you are not using Docker, make sure MongoDB and Redis are running on their default ports.

## Running the Application

### Start the server

```bash
cd server
npm start
```

The server listens on port 3000 by default.

### Run the worker

From the project root:

```bash
./build/judge_worker
```

## API Usage

### Submit a job

```bash
curl -X POST http://localhost:3000/submit \
  -H "Content-Type: application/json" \
  -d '{
    "code": "print(1)",
    "input": "",
    "expected": "1",
    "language": "python"
  }'
```

### Check job status

```bash
curl http://localhost:3000/jobs/<job-id>
```

## Notes

The server uses environment variables for configuration:

- MONGO_URI
- MONGO_DB_NAME
- REDIS_URL
- QUEUE_NAME
- PORT

You can define them in a .env file at the project root.
