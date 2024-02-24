#pragma once

#include "print.hpp"

#include <memory>
#include <queue>
#include <sstream>
#include <string>

class CommandQueue {
  private:
    std::queue<std::string> fifo_;
    std::shared_ptr<IPrintable> printer_;

  public:
    explicit CommandQueue(std::shared_ptr<IPrintable> printer)
        : printer_(std::move(printer)) {}

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

        printer_->write(out.str());
    }
};
