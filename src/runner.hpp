#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include "async/mq.hpp"
#include "processor.hpp"

class Runner {
  private:
    CommandProcessor proc_;
    otus::StringMQ mqueue_;
    std::string async_buffer;

  public:
    Runner(int blockSize)
        : proc_(CommandProcessor(blockSize)),
          mqueue_("async", otus::StringMQ::EndpointType::Server, 10, 1024) {}

    bool DoWork() {
        auto async_callback = [this](const std::string &message) {
            async_buffer += message;
            if (message.find('\n') == std::string::npos) {
                return;
            }

            std::string filtered;
            std::stringstream sstream(async_buffer);
            while (std::getline(sstream, filtered, '\n')) {
                proc_.push(filtered);
            }
            async_buffer = async_buffer.substr(async_buffer.find_last_of('\n') + 1);
        };
        mqueue_.attach(async_callback);

        std::string line;
        while (std::getline(std::cin, line)) {
            proc_.push(line);
        }
        return true;
    }
};
