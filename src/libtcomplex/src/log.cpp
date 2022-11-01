#include "log.h"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <mutex>

#include <tsl/robin_map.h>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>

namespace libtcomplex::log {

constexpr const char *console_handle_key = "console";
constexpr const char *file_handle_key = "file";

static std::mutex reset_mutex;

formatter::condition_pattern_formater *ptr_log_formatter() {
    static auto log_formatter = std::make_unique<formatter::condition_pattern_formater>(
        std::make_unique<spdlog::pattern_formatter>("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%s:%#:%!] %v"));
    return log_formatter.get();
}

void reset_logging(const log_param_t log_param) {
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
                auto ptr_formatter = ptr_log_formatter();
                console_sink->set_formatter(ptr_formatter->clone());
                sink_registry[console_handle_key] = console_sink;
            }
            sink_vec.push_back(sink_registry.at(console_handle_key));
        }

        if (log_type == log_type_t::file_only || log_type == log_type_t::console_file) {
            if (std::size(log_filename) > 0) {
                if (!sink_registry.contains(file_handle_key)) {
                    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_filename, true);
                    file_sink->set_level(spdlog::level::trace);
                    auto ptr_formatter = ptr_log_formatter();
                    file_sink->set_formatter(ptr_formatter->clone());
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
    std::scoped_lock lock(reset_mutex);
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

std::shared_ptr<spdlog::logger> get_logger(const std::string_view name, const std::string_view pattern) {
    std::scoped_lock lock(reset_mutex);
    auto logger = spdlog::get(std::string{name});
    // clone a new formatter
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
            auto ptr_formatter = ptr_log_formatter();
            auto pattern_formatter_ = std::make_unique<spdlog::pattern_formatter>(std::string{pattern});
            ptr_formatter->add_condition(name, std::move(pattern_formatter_));
            logger->set_formatter(ptr_formatter->clone());
        }
    } else {
        auto ptr_formatter = ptr_log_formatter();
        auto pattern_formatter_ = std::make_unique<spdlog::pattern_formatter>(std::string{pattern});
        ptr_formatter->add_condition(name, std::move(pattern_formatter_));
        logger->set_formatter(ptr_formatter->clone());
    }
    return logger;
}

namespace formatter {

std::unique_ptr<spdlog::formatter> condition_pattern_formater::clone() const {
    auto new_default = formatter_default->clone();
    condition_pattern_formater::formatter_map new_formatter_map;
    for (auto &it : formatter_map_) {
        new_formatter_map.insert({it.first, std::move(it.second->clone())});
    }
    return std::make_unique<condition_pattern_formater>(std::move(new_default), std::move(new_formatter_map));
}

void condition_pattern_formater::format(const spdlog::details::log_msg &msg, spdlog::memory_buf_t &dest) {
    const auto _src_name = msg.logger_name;
    const std::string_view src_name = {_src_name.data(), _src_name.size()};
    if (formatter_map_.contains(src_name)) {
        const auto ptr_formatter = formatter_map_.at(src_name).get();
        ptr_formatter->format(msg, dest);
    } else {
        // use default
        formatter_default->format(msg, dest);
    }
}

void condition_pattern_formater::add_condition(const std::string_view name, std::unique_ptr<spdlog::formatter> f) {
    if (formatter_map_.contains(name)) {
        formatter_map_.erase(name);
    }
    formatter_map_.insert({std::string{name}, std::move(f)});
}

} // namespace formatter

} // namespace libtcomplex::log