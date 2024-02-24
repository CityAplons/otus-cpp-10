#pragma once

#include "print.hpp"

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <queue>
#include <thread>

template <typename T> class SQueue {
  public:
    void push(const T &data) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(data));
        }
        sync_.notify_one();
    }

    bool try_pop(T &data) {
        std::unique_lock<std::mutex> lock(mutex_);
        auto timeout = std::chrono::milliseconds(100);
        if (!sync_.wait_for(lock, timeout,
                            [this] { return !queue_.empty(); })) {
            return false;
        }

        data = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

  private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable sync_;
};

template <typename action, size_t pool_size = 1>
class ThreadedPrintable : public IPrintable {
  private:
    SQueue<std::string> queue_;
    std::array<std::thread, pool_size> threads_;
    std::atomic_bool stop_ = false;

    void do_work(void) {
        action work;
        while (!stop_) {
            std::string data;
            if (queue_.try_pop(data)) {
                work(data);
            }
        }
    }

  public:
    ThreadedPrintable() {
        for (auto &&thread : threads_) {
            thread = std::thread(&ThreadedPrintable::do_work, this);
        }
    }

    ~ThreadedPrintable() {
        // Await for threads to do their work
        while (queue_.size()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Stop threads
        stop_ = true;
        for (auto &&thread : threads_) {
            thread.join();
        }
    }

    void write(const std::string &data) final { queue_.push(data); }
};

struct FilePrintFunctor {
   void operator()(const std::string &data) {
        auto thread_id =
            std::hash<std::thread::id>{}(std::this_thread::get_id());

        using namespace std::chrono;
        const auto stamp = current_zone()->to_local(system_clock::now());
        const auto unixStamp =
            duration_cast<seconds>(stamp.time_since_epoch()).count();

        int i = 0;
        std::string fileName = std::format(
            "bulk{}_{:x}.log", std::to_string(unixStamp), thread_id);
        while (std::filesystem::exists(fileName)) {
            ++i;
            fileName = std::format("bulk{}_{:x}_{}.log",
                                   std::to_string(unixStamp), thread_id, i);
        }

        std::ofstream file(fileName);
        file << data;
        std::flush(file);
        file.close();
    }
};

struct ConsolePrintFunctor {
  void operator()(const std::string &data) {
        std::cout << data;
        std::flush(std::cout);
    }
};
