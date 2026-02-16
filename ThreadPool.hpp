#pragma once
#include "BlockingQueue.hpp" 
#include <vector>
#include <thread>
#include <functional>
#include <atomic>
#include <optional>
#include <chrono>
#include <string>

struct Task {
    std::string filepath;
};

class ThreadPool {
private:
    BlockingQueue<Task> taskQueue;
    BlockingQueue<int> resultQueue;
    std::vector<std::thread> workers;
    std::atomic<bool> stopFlag{false};
    std::atomic<int> activeWorkers{0};
    std::atomic<int> tasksCompleted{0};
    std::atomic<int> tasksSubmitted{0};
    std::function<int(const std::string&)> processFunc;
    
public:
    ThreadPool(size_t numThreads, 
               std::function<int(const std::string&)> func) 
        : processFunc(func) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this]() {
                activeWorkers++;
                while (!stopFlag) {
                    auto taskOpt = taskQueue.pop();
                    if (!taskOpt) break;
                    
                    Task task = std::move(*taskOpt);
                    int result = processFunc(task.filepath);
                    resultQueue.push(result);
                    tasksCompleted++;
                }
                activeWorkers--;
            });
        }
    }
    
    void submit(Task task) {
        if (!stopFlag) {
            taskQueue.push(std::move(task));
            tasksSubmitted++;
        }
    }
    
    std::optional<int> getResult() {
        return resultQueue.pop();
    }

    std::vector<int> waitAllResults() {
        std::vector<int> results;
        while (tasksCompleted < tasksSubmitted) {
            auto resultOpt = resultQueue.pop();
            if (resultOpt) {
                results.push_back(*resultOpt);
            }
        }
        return results;
    }
    
    void stop() {
        stopFlag = true;
        taskQueue.stop();
    }
    
    ~ThreadPool() {
        stop();
        for (auto& worker : workers) {
            if (worker.joinable()) worker.join();
        }
    }
    size_t pendingTasks() const { 
        return tasksSubmitted - tasksCompleted; 
    }
    size_t completedTasks() const { 
        return tasksCompleted; 
    }
};