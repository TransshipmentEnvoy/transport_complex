#include <spdlog/spdlog.h>

#include <chrono>
#include <thread>

#include <libtcomplex/lib.h>

#include "log.h"

namespace libtcomplex {

int run() {
    log::reset_logging();

    const char *str = "hello, world";
    spdlog::info("{}!!", str);

    auto root = spdlog::get("root");
    auto tmp = log::get_logger("tmp");

    for (int i = 0; i < 10; i++) {
        tmp->info("{}  --", i);
        spdlog::info("{}  ^^", 10 - i);
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }

    log::ref_log_type() = log::log_type_t::file_only;
    log::reset_logging();
    spdlog::info("{}!!", str);

    return 0;
}

void init() { log::reset_logging(); }

void deinit() {}

} // namespace libtcomplex