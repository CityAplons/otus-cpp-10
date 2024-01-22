#pragma once

#include <chrono>
#include <cstdlib>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace threaded {

template <size_t pool_size = 1> class Pool {
  private:
    std::array<std::thread, pool_size> threads_;

  public:
    void run(std::function<void(const std::string &)> task,
             const std::string &data) {
        // trivial impl, async seems to be not using the thread pool
        for (auto &&thread : threads_) {
            if (thread.joinable())
                continue;

            thread = std::thread(task, data);
            return;
        }

        for (auto &&thread : threads_) {
            if (!thread.joinable())
                continue;

            thread.join();
            thread = std::thread(task, data);
            return;
        }
    }

    ~Pool() {
        for (auto &&thread : threads_) {
            if (!thread.joinable())
                continue;

            thread.join();
        }
    };
};

class IPrintable {
  public:
    virtual void write(const std::string &) = 0;
    virtual ~IPrintable() {}
};

class FilePrint : public IPrintable {
  private:
    Pool<2> workers_;

  public:
    void write(const std::string &data) final {
        auto task = [](const std::string &local_data) {
            using namespace std::chrono;
            const auto stamp = current_zone()->to_local(system_clock::now());
            const auto unixStamp =
                duration_cast<seconds>(stamp.time_since_epoch()).count();
            std::string fileName = std::format(
                "bulk{}_{:x}.log", std::to_string(unixStamp), std::rand());

            std::ofstream file(fileName);
            file << local_data;

            file.flush();
            file.close();
        };
        workers_.run(task, data);
    };
};

class ConsolePrint : public IPrintable {
  private:
    Pool<1> workers_;

  public:
    void write(const std::string &data) final {
        auto task = [](const std::string &local_data) {
            std::cout << local_data;
            std::flush(std::cout);
        };
        workers_.run(task, data);
    };
};

}   // namespace threaded

struct Print {
    std::vector<std::ostream *> streams;
    void AddStream(std::ostream &stream) { streams.push_back(&stream); }
    void RemoveStream() { streams.pop_back(); }
};

template <class T>
Print &
operator<<(Print &fork, T data) {
    for (auto &&stream : fork.streams) {
        *stream << data;
    }
    return fork;
}

Print &
operator<<(Print &fork, std::ostream &(*f)(std::ostream &) ) {
    for (auto &&stream : fork.streams) {
        *stream << f;
    }
    return fork;
}

template <class T>
Print &
operator<<(Print &fork, std::ostream &(*f)(std::ostream &, T)) {
    for (auto &&stream : fork.streams) {
        *stream << f;
    }
    return fork;
}