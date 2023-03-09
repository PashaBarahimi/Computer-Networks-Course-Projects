// This is not part of the CLI library and is added by Pasha and Misagh to
// handle the input from the user inside a command. This class makes it
// possible to have a custom queue for the terminal to post the input from
// the command handler instead of the scheduler.

#ifndef CONCURRENT_QUEUE_H_
#define CONCURRENT_QUEUE_H_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace concurrent {

template <typename T>
class Queue {
public:
    Queue()
        : running_(true) {}

    ~Queue() {
        running_ = false;
        cv_.notify_one();
    }

    void push(const T& t) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(t);
        cv_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() { return !queue_.empty() || !running_; });
        if (!running_) {
            return T();
        }
        T t = std::move(queue_.front());
        queue_.pop();
        return t;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!queue_.empty()) {
            queue_.pop();
        }
    }

private:
    bool running_;
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

} // namespace concurrent

#endif // CONCURRENT_QUEUE_H_
