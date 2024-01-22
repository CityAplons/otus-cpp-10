#include "runner.hpp"

#include <cassert>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    assert(argc > 1);

    int blockSize = std::atoi(argv[1]);
    assert(blockSize > 0);

    Runner{blockSize}.DoWork();
    return 0;
}
