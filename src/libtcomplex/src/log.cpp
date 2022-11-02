#include "log.h"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <mutex>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/map.hpp>

#include <tsl/robin_map.h>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>

namespace libtcomplex::log {

static std::mutex reset_mutex;

formatter::condition_pattern_formatter *ptr_log_formatter() { // todo: support dynamic sink
    static auto log_formatter = std::make_unique<formatter::condition_pattern_formatter>(
        std::make_unique<spdlog::pattern_formatter>("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%s:%#:%!] %v"));
    return log_formatter.get();
}

// reset_logging, core of logging device
void reset_logging(const log_param_t log_param) {
    static bool thread_pool_inited = false;
    static log_type_t log_type = log_type_t::disabled;
    static std::string log_filename;

    // lock
    std::scoped_lock lock(reset_mutex);

    // load ctx
    sink_registry &sink_registry_ = ref_sink_registry();

    // update global log_param
    if (log_param.log_type.has_value()) {
        log_type = log_param.log_type.value();
    }
    if (log_param.log_filename.has_value()) {
        // uninstall sink
        sink_registry_.uninstall_sink(file_sink_key);
        // update tmp_filename
        auto tmp_filename = log_param.log_filename.value();
        log_filename = std::string{tmp_filename.cbegin(), tmp_filename.cend()};
        // unreg sink
        sink_registry_.unreg_sink(file_sink_key);
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

    // setup logging!
    try {
        sink_registry_.reg_default(log_type, log_filename);
    } catch (const spdlog::spdlog_ex &ex) {
        fprintf(stderr, "error! %s\n", ex.what());
    }

    // going internal! (waiting for refactor)
    // for each regged sink, search if has installed sink
    // if not, use init fn create a new one and set with installed formatter
    std::vector<spdlog::sink_ptr> sink_vec;
    try {
        if (!thread_pool_inited) {
            spdlog::init_thread_pool(ASYNC_QUEUE_SIZE, ASYNC_THREAD_COUNT);
            thread_pool_inited = true;
        }

        sink_registry_.upkeep();

        sink_vec = sink_registry_.sink_items() | ranges::views::values | ranges::to<std::vector>;

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

// sink registry
sink_registry::sink_registry() {
    default_formatter = std::move(std::make_unique<formatter::condition_pattern_formatter>(
        std::make_unique<spdlog::pattern_formatter>("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%s:%#:%!] %v")));
}

void sink_registry::reg_default(const log_type_t log_type, const std::string_view log_filename) {
    // reg default sink if not regged
    if (log_type == log_type_t::console_only || log_type == log_type_t::console_file) {
        if (!this->check_sink_reg(console_sink_key)) {
            this->reg_sink(console_sink_key, []() {
                auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                console_sink->set_level(spdlog::level::info);
                return console_sink;
            });
        }
    } else {
        if (this->check_sink_reg(console_sink_key)) {
            this->unreg_sink(console_sink_key);
        }
    }

    if ((log_type == log_type_t::file_only || log_type == log_type_t::console_file) && (std::size(log_filename) > 0)) {
        if (!this->check_sink_reg(file_sink_key)) {
            const std::string log_filename_curr{log_filename};
            this->reg_sink(console_sink_key, [log_filename_curr]() -> spdlog::sink_ptr {
                try {
                    if (std::size(log_filename_curr) > 0) {
                        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_filename_curr, true);
                        file_sink->set_level(spdlog::level::trace);
                        return file_sink;
                    } else {
                        return nullptr;
                    }
                } catch (const spdlog::spdlog_ex &ex) {
                    return nullptr;
                }
            });
        }
    } else {
        if (this->check_sink_reg(file_sink_key)) {
            this->unreg_sink(file_sink_key);
        }
    }

    // reg default formatter if not regged
    if (!this->check_sink_formatter(console_sink_key)) {
        this->install_sink_formatter(
            console_sink_key,
            std::move(std::make_unique<formatter::condition_pattern_formatter>(
                std::make_unique<spdlog::pattern_formatter>("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%s:%#:%!] %v"))));
    }
    if (!this->check_sink_formatter(file_sink_key)) {
        this->install_sink_formatter(
            file_sink_key,
            std::move(std::make_unique<formatter::condition_pattern_formatter>(
                std::make_unique<spdlog::pattern_formatter>("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%s:%#:%!] %v"))));
    }
}

void sink_registry::upkeep() {
    // get a const ref of current installed sink & formatter
    const auto &sink_map_curr = this->sink_map_;
    auto &sink_formatter_map_curr = this->sink_formatter_map_;
    sink_registry::sink_map new_sink_map;

    for (const auto iter_pair : this->sink_init_fn_map_) {
        const std::string_view sink_key = iter_pair.first;
        // there will be 3 valid condition
        // active sink + active formatter
        // no sink + active formatter
        // no sink + no formatter

        // install sink!
        if (sink_map_curr.contains(sink_key)) {
            new_sink_map.insert({std::string{sink_key}, sink_map_curr.at(sink_key)});
        } else {
            auto sink_init_curr = iter_pair.second;
            auto new_sink = sink_init_curr();
            if (new_sink != nullptr) {
                if (sink_formatter_map_curr.contains(sink_key)) {
                    // install existing formatter (left behind from past regged sink?)
                    auto _curr_formatter = sink_formatter_map_curr.at(sink_key).get();
                    new_sink->set_formatter(_curr_formatter->clone());
                } else {
                    // use default formatter
                    sink_formatter_map_curr.insert({std::string{sink_key}, default_formatter->exact_clone()});
                    new_sink->set_formatter(default_formatter->clone());
                }
                new_sink_map.insert({std::string{sink_key}, new_sink});
            } else {
                // skip build this sink, but record formatter
                if (!sink_formatter_map_curr.contains(sink_key)) {
                    // use default formatter
                    sink_formatter_map_curr.insert({std::string{sink_key}, default_formatter->exact_clone()});
                }
            }
        }
    }

    // move assign to sink_registry
    this->sink_map_ = std::move(new_sink_map);
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

std::shared_ptr<spdlog::logger> get_logger(const std::string_view name, sink_formatter_map fmap) {
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
        }
    }
    // if logger is created, set sink formatters
    if (logger != nullptr) {
        sink_registry &sink_registry_ = ref_sink_registry();
        for (auto sink_key : fmap | ranges::views::keys) {
            auto target_f = sink_registry_.get_formatter(sink_key);
            auto mode_f = fmap.at(sink_key).get();
            target_f->add_condition(name, mode_f->clone());
            // override sink formatter
            if (sink_registry_.check_sink_install(sink_key)) {
                auto sink_p = sink_registry_.get_sink(sink_key);
                sink_p->set_formatter(target_f->clone());
            }
        }
    }

    return logger;
}

namespace formatter {

std::unique_ptr<spdlog::formatter> condition_pattern_formatter::clone() const { return this->exact_clone(); }

void condition_pattern_formatter::format(const spdlog::details::log_msg &msg, spdlog::memory_buf_t &dest) {
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

void condition_pattern_formatter::add_condition(const std::string_view name, std::unique_ptr<spdlog::formatter> f) {
    if (formatter_map_.contains(name)) {
        formatter_map_.erase(name);
    }
    formatter_map_.insert({std::string{name}, std::move(f)});
}

std::unique_ptr<condition_pattern_formatter> condition_pattern_formatter::exact_clone() const {
    auto new_default = formatter_default->clone();
    condition_pattern_formatter::formatter_map new_formatter_map;
    for (auto &it : formatter_map_) {
        new_formatter_map.insert({it.first, std::move(it.second->clone())});
    }
    return std::make_unique<condition_pattern_formatter>(std::move(new_default), std::move(new_formatter_map));
}

} // namespace formatter

} // namespace libtcomplex::log