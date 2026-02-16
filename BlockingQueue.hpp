#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class BlockingQueue {
private:
    std::queue<T> q;
    mutable std::mutex m;
    std::condition_variable c;
    bool stopped = false;

public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(m);
        if (stopped) return;
        q.push(std::move(value));
        c.notify_one();
    }

    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(m);
        c.wait(lock, [this]() { return !q.empty() || stopped; });
        
        if (stopped && q.empty()) {
            return std::nullopt;
        }
        T value = std::move(q.front());
        q.pop();
        return value;
    }

    void stop() {
        std::lock_guard<std::mutex> lock(m);
        stopped = true;
        c.notify_all();
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m);
        return q.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(m);
        return q.size();
    }
};