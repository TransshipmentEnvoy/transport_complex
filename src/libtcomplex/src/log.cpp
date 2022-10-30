#include "log.h"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <mutex>
#include <tsl/robin_map.h>

namespace libtcomplex::log {

void reset_logging(const std::optional<log_type_t> log_type_in) {
    static std::mutex reset_mutex;
    static tsl::robin_map<std::string, spdlog::sink_ptr> sink_registry;
    static bool thread_pool_inited = false;
    static log_type_t log_type = log_type_t::disabled;

    if (log_type_in.has_value()) {
        log_type = log_type_in.value();
    }

    // lock
    std::scoped_lock lock(reset_mutex);

    // force all loggers to flush
    try {
        spdlog::apply_all([](const std::shared_ptr<spdlog::logger> l) { l->flush(); });
    } catch (const spdlog::spdlog_ex &ex) {
        fprintf(stderr, "error! %s\n", ex.what());
    }

    // remove default logger
    try {
        spdlog::set_default_logger(nullptr);
    } catch (const spdlog::spdlog_ex &ex) {
        fprintf(stderr, "error! %s\n", ex.what());
    }

    // get existing registered logger
    std::vector<std::shared_ptr<spdlog::logger>> logger_vec;
    try {
        spdlog::apply_all([&](const std::shared_ptr<spdlog::logger> l) { logger_vec.push_back(l); });
    } catch (const spdlog::spdlog_ex &ex) {
        fprintf(stderr, "error! %s\n", ex.what());
    }

    // drop existing log device
    try {
        spdlog::drop_all();
    } catch (const spdlog::spdlog_ex &ex) {
        fprintf(stderr, "error! %s\n", ex.what());
    }

    // setup logging
    std::vector<spdlog::sink_ptr> sink_vec;
    try {
        if (!thread_pool_inited) {
            spdlog::init_thread_pool(ASYNC_QUEUE_SIZE, ASYNC_THREAD_COUNT);
            thread_pool_inited = true;
        }

        if (log_type == log_type_t::console_only || log_type == log_type_t::console_file) {
            if (!sink_registry.contains("console")) {
                auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                console_sink->set_level(spdlog::level::info);
                console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");
                sink_registry["console"] = console_sink;
            }
            sink_vec.push_back(sink_registry.at("console"));
        }

        if (log_type == log_type_t::file_only || log_type == log_type_t::console_file) {
            if (!sink_registry.contains("file")) {
                auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("multisink.txt", true);
                file_sink->set_level(spdlog::level::trace);
                sink_registry["file"] = file_sink;
            }
            sink_vec.push_back(sink_registry.at("file"));
        }

        spdlog::set_default_logger(
            std::make_shared<spdlog::async_logger>(std::string{ROOT_LOGGER_NAME}, sink_vec.cbegin(), sink_vec.cend(),
                                                   spdlog::thread_pool(), spdlog::async_overflow_policy::block));

    } catch (const spdlog::spdlog_ex &ex) {
        fprintf(stderr, "error! %s\n", ex.what());
    }

    // for each logger: use new sinks & re-register
    try {
        for (const auto l : logger_vec) {
            l->sinks() = sink_vec;
            spdlog::register_logger(l);
        }
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
            logger = std::make_shared<spdlog::async_logger>(std::string{name}, sinks.cbegin(), sinks.cend(),
                                                            spdlog::thread_pool());
            logger->set_level(root->level());
            spdlog::register_logger(logger);
        }
    }
    return logger;
}
} // namespace libtcomplex::log