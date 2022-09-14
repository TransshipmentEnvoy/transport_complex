#include <spdlog/spdlog.h>

#include <chrono>
#include <thread>

#include <libtcomplex/lib.h>

#include "log.h"

namespace libtcomplex {

int run() {
    log::setup_logging();

    const char *str = "hello, world";
    spdlog::info("{}!!", str);

    auto root = spdlog::get("root");
    auto tmp = log::get_logger("tmp");

    for (int i = 0; i < 100; i++) {
        tmp->info("{}  --", i);
        spdlog::info("{}  ^^", 100 - i);
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }

    return 0;
}

} // namespace libtcomplex