#include "async.hpp"
#include "lib_runner.hpp"

#include <format>
#include <limits.h>
#include <unistd.h>

namespace async {

handle_t
connect(std::size_t bulk) {
    return static_cast<void *>(otus::ConcurrentRunner::Get().connect(bulk));
}

void
receive(handle_t handle, const char *data, std::size_t size) {
    if (handle == nullptr) {
        return;
    }

    auto priv = static_cast<otus::ConcurrentRunner::runner_t>(handle);
    otus::ConcurrentRunner::Get().receive(priv, data, size);
}

void
disconnect(handle_t handle) {
    // TODO: pass handle_t by a reference to be able to invalidate it !!!
    if (handle == nullptr) {
        return;
    }

    auto priv = static_cast<otus::ConcurrentRunner::runner_t>(handle);
    otus::ConcurrentRunner::Get().disconnect(priv);
}

}   // namespace async
