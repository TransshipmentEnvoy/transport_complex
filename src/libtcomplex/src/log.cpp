#include "log.h"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <mutex>

#include <tsl/robin_map.h>

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/chrono.h>

namespace libtcomplex::log {

constexpr const char *console_handle_key = "console";
constexpr const char *file_handle_key = "file";

void reset_logging(const log_param_t log_param) {
    static std::mutex reset_mutex;
    static tsl::robin_map<std::string, spdlog::sink_ptr> sink_registry;
    static bool thread_pool_inited = false;
    static log_type_t log_type = log_type_t::disabled;
    static std::string log_filename;

    // lock
    std::scoped_lock lock(reset_mutex);

    // update global log_param
    if (log_param.log_type.has_value()) {
        log_type = log_param.log_type.value();
    }
    if (log_param.log_filename.has_value()) {
        // drop existing file handler
        sink_registry.erase(file_handle_key);
        // update tmp_filename
        auto tmp_filename = log_param.log_filename.value();
        log_filename = std::string{tmp_filename.cbegin(), tmp_filename.cend()};
    }

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
            if (!sink_registry.contains(console_handle_key)) {
                auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                console_sink->set_level(spdlog::level::info);
                console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");
                sink_registry[console_handle_key] = console_sink;
            }
            sink_vec.push_back(sink_registry.at(console_handle_key));
        }

        if (log_type == log_type_t::file_only || log_type == log_type_t::console_file) {
            if (std::size(log_filename) > 0) {
                if (!sink_registry.contains(file_handle_key)) {
                    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_filename, true);
                    file_sink->set_level(spdlog::level::trace);
                    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v");
                    sink_registry[file_handle_key] = file_sink;
                }
                sink_vec.push_back(sink_registry.at(file_handle_key));
            }
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