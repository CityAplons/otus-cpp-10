#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include "log.hpp"
#include "processor.hpp"

class Runner {
  private:
    std::unique_ptr<CommandProcessor> proc_;

  public:
    Runner(int blockSize) {
        auto printer = std::make_shared<PrintComposite>();
        printer->Add(std::make_shared<FilePrint>());
        printer->Add(std::make_shared<ConsolePrint>());

        proc_ = std::make_unique<CommandProcessor>(blockSize, printer);
    }

    bool DoWork() {
        std::string line;
        while (std::getline(std::cin, line)) {
            proc_->push(line);
        }
        return true;
    }
};
