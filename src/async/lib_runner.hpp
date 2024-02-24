#pragma once

#include <map>
#include <sstream>
#include <string>

#include "../log.hpp"
#include "../print_threaded.hpp"
#include "../processor.hpp"

namespace otus {

struct Client {
    std::unique_ptr<CommandProcessor> proc;
    std::string buffer;
};

class ConcurrentRunner {
  private:
    std::map<int, Client> clients_;
    std::shared_ptr<PrintComposite> printer_;

    ConcurrentRunner() : printer_(std::make_shared<PrintComposite>()) {
        printer_->Add(std::make_shared<ThreadedPrintable<FilePrintFunctor, 2>>());
        printer_->Add(std::make_shared<ThreadedPrintable<ConsolePrintFunctor>>());
    }

    ~ConcurrentRunner() {
        otus::Log::Get().Debug("dtor {}", get_type_name<decltype(this)>()); 
    }

    ConcurrentRunner(const ConcurrentRunner &root) = delete;
    ConcurrentRunner &operator=(const ConcurrentRunner &) = delete;
    ConcurrentRunner(ConcurrentRunner &&root) = delete;
    ConcurrentRunner &operator=(ConcurrentRunner &&) = delete;

  public:
    using runner_t = int *;

    static ConcurrentRunner &Get() {
        static ConcurrentRunner instance;
        return instance;
    }

    runner_t connect(size_t bulk) {
        runner_t handle = new int(0);
        auto latest_client = clients_.rbegin();
        if (latest_client != clients_.rend()) {
            *handle = latest_client->first + 1;
        }

        int id = *handle;
        auto proc = std::make_unique<CommandProcessor>(bulk, printer_);
        clients_[id] = {std::move(proc), ""};
        otus::Log::Get().Debug("(Connect) New [{}] client connected", id);
        return handle;
    }

    void disconnect(runner_t handle) {
        if (!handle || !clients_.count(*handle)) {
            otus::Log::Get().Error("(Disconnect) Bad handle");
            return;
        }

        int id = *handle;
        otus::Log::Get().Debug("(Disconnect) [{}] client disconnected", id);
        clients_.erase(id);
        delete[] handle;
    }

    bool receive(runner_t handle, const char *data, size_t len) {
        if (!handle || !clients_.count(*handle)) {
            otus::Log::Get().Error("(Receive) Bad handle");
            return false;
        }

        int id = *handle;
        std::string received(data, data + len);
        clients_[id].buffer += received;
        if (received.find('\n') == std::string::npos) {
            otus::Log::Get().Debug("(Receive) chunk received");
            return true;
        }

        std::string filtered;
        std::stringstream sstream(clients_[id].buffer);
        while (std::getline(sstream, filtered, '\n')) {
            clients_[id].proc->push(filtered);
        }

        auto reminder_begin = clients_[id].buffer.find_last_of('\n') + 1;
        clients_[id].buffer = clients_[id].buffer.substr(reminder_begin);
        return true;
    }

    size_t clients_amount() { return clients_.size(); }
};

}   // namespace otus
