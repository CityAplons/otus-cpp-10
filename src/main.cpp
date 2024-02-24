#include "runner.hpp"

#include <cassert>
#include <iostream>
#include <string>

int
main(int argc, char *argv[]) {
    otus::Log::Get().SetSeverity(otus::Log::INFO);
    if (argc < 2) {
        otus::Log::Get().Info("Provide block size!\n\tUsage:{} [BLOCK_SIZE]", argv[0]);
        return 0;
    }

    int blockSize = std::atoi(argv[1]);
    if (blockSize == 0) {
        otus::Log::Get().Error("Bad block size provided");
        return 1;
    }

    Runner{blockSize}.DoWork();
    return 0;
}
