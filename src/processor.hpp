#pragma once

#include "bulk.hpp"
#include "log.hpp"

class CommandProcessor {
  public:
    explicit CommandProcessor(int blockSize,
                              std::shared_ptr<IPrintable> printer)
        : blockSize_(blockSize), queue_(printer) {
        otus::Log::Get().Info("Bulk created with new block size {}",
                              blockSize_);
    }

    ~CommandProcessor() {
        if (depth_ == 0) {
            queue_.ExecuteAll();
        }
    }

    void push(const std::string &line) {
        if (!line.size()) {
            return;
        }

        ++counter_;
        if (line.compare("{") == 0) {
            counter_ = 0;

            if (depth_ == 0) {
                queue_.ExecuteAll();
            }
            ++depth_;
        } else if (line.compare("}") == 0) {
            counter_ = 0;

            --depth_;
            if (depth_ == 0) {
                queue_.ExecuteAll();
            }
        } else if (line[0] == '=') {
            // Set new block-size by command
            counter_ = 0;
            blockSize_ = std::abs(std::atoi(&line[1]));

            std::string info = std::format("Set-size {}", blockSize_);
            otus::Log::Get().Warn(info);
        } else {
            queue_.Add(line);
        }

        if (counter_ == blockSize_) {
            queue_.ExecuteAll();
            counter_ = 0;
        }
    }

  private:
    int counter_ = 0, depth_ = 0;
    int blockSize_ = 1;

    CommandQueue queue_;
};