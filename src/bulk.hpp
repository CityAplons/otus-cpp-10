#pragma once

#include "command.hpp"
#include "print.hpp"

#include <memory>
#include <queue>
#include <sstream>
#include <string>

class CommandQueue {
  private:
    std::queue<std::string> fifo_;

  public:
    std::string &GetLastCmd() { return fifo_.front(); }
    void Add(const std::string &cmd) {
        if (cmd.size() == 0) {
            return;
        }

        fifo_.push(cmd);
    }

    void ExecuteAll() {
        if (fifo_.size() == 0) {
            return;
        }

        std::stringstream out;
        out << "bulk:";
        while (!fifo_.empty()) {
            out << ' ' << fifo_.front();
            fifo_.pop();
        }
        out << '\n';

        threaded::FilePrint().write(out.str());
        threaded::ConsolePrint().write(out.str());
    }
};

class OnBulkAppend : public Command {
  private:
    std::shared_ptr<CommandQueue> queue_;
    std::string data_;

  public:
    OnBulkAppend(std::shared_ptr<CommandQueue> &queue, const std::string &cmd)
        : queue_(queue), data_(cmd) {}
    void Execute() override { queue_->Add(data_); }
};

class OnBulkFlush : public Command {
  private:
    std::shared_ptr<CommandQueue> queue_;

  public:
    OnBulkFlush(std::shared_ptr<CommandQueue> &queue) : queue_(queue) {}
    void Execute() override { queue_->ExecuteAll(); }
};