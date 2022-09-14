#include "log.h"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace libtcomplex::log {
void setup_logging() {
    try {
        spdlog::init_thread_pool(8192, 1);

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);
        console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("multisink.txt", true);
        file_sink->set_level(spdlog::level::trace);

        spdlog::set_default_logger(
            std::make_shared<spdlog::async_logger>("root", spdlog::sinks_init_list({console_sink, file_sink}),
                                                   spdlog::thread_pool(), spdlog::async_overflow_policy::block));

    } catch (const spdlog::spdlog_ex &ex) {
        fprintf(stderr, "error! %s\n", ex.what());
    }
}

// get logger
std::shared_ptr<spdlog::logger> get_logger(const std::string_view name) {
    auto logger = spdlog::get(std::string{name});
    if (logger == nullptr) {
        auto root = spdlog::get("root");
        if (root == nullptr) {
            logger = nullptr;
        } else {
            // create a new logger using same sink with root
            auto &sinks = root->sinks();
            logger = std::make_shared<spdlog::async_logger>(std::string{name}, sinks.begin(), sinks.end(),
                                                            spdlog::thread_pool());
            logger->set_level(root->level());
            spdlog::register_logger(logger);
        }
    }
    return logger;
}
} // namespace libtcomplex::log