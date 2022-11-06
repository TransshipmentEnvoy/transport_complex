#include "log.h"

#include <spdlog/async.h>
#include <spdlog/details/fmt_helper.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <mutex>
#include <thread>

#include <range/v3/range/conversion.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/subrange.hpp>

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
        sink_registry_.wipe_sink(file_sink_key);
        // update tmp_filename
        auto tmp_filename = log_param.log_filename.value();
        log_filename = std::string{tmp_filename.cbegin(), tmp_filename.cend()};
        // unreg sink
        sink_registry_.unreg_sink(file_sink_key);
    }

    // force all loggers to flush
    flush_logging();

    // clear all loggers sinks
    try {
        spdlog::apply_all([](const std::shared_ptr<spdlog::logger> l) { l->sinks().clear(); });
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
        sink_registry_.reg_default(log_filename);
    } catch (const spdlog::spdlog_ex &ex) {
        fprintf(stderr, "error! %s\n", ex.what());
    }

    //
    std::vector<spdlog::sink_ptr> sink_vec;
    try {
        if (!thread_pool_inited) {
            spdlog::init_thread_pool(ASYNC_QUEUE_SIZE, ASYNC_THREAD_COUNT);
            thread_pool_inited = true;
        }

        sink_registry_.upkeep_default(log_type);

        sink_vec = sink_registry_.sink_items() | ranges::views::values | ranges::to<std::vector>;

        auto root_logger =
            std::make_shared<spdlog::async_logger>(std::string{ROOT_LOGGER_NAME}, sink_vec.cbegin(), sink_vec.cend(),
                                                   spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        root_logger->set_level(spdlog::level::trace);
        spdlog::set_default_logger(root_logger);

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

// flush logging
void flush_logging() {
    // force all loggers to flush
    try {
        spdlog::apply_all([](const std::shared_ptr<spdlog::logger> l) { l->flush(); });
    } catch (const spdlog::spdlog_ex &ex) {
        fprintf(stderr, "error! %s\n", ex.what());
    }

    // wait until threadpool is empty
    try {
        auto spdlog_tp = spdlog::thread_pool();
        if (spdlog_tp != nullptr) {
            while (spdlog_tp->queue_size() != 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(FLUSH_WAIT_SLICE));
            }
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

// buf sink
void sink_registry::enable_sink(const std::string_view sink_key) {
    if (!this->check_sink_reg(sink_key)) {
        return;
    }

    if (!this->check_sink_buf(sink_key)) {
        auto &sink_init_curr = this->sink_reg_map_.at(sink_key);
        auto sink_curr = sink_init_curr();
        if (sink_curr == nullptr) {
            return;
        }

        // set formatter
        if (!this->check_sink_formatter(sink_key)) {
            this->sink_formatter_map_.insert({std::string{sink_key}, this->default_formatter->exact_clone()});
            sink_curr->set_formatter(this->default_formatter->clone());
        } else {
            auto formatter_curr = this->sink_formatter_map_.at(sink_key).get();
            sink_curr->set_formatter(formatter_curr->clone());
        }

        // store in buf
        this->sink_buf_map_.insert({std::string{sink_key}, sink_curr});
    }

    if (!this->check_sink_install(sink_key)) {
        this->sink_map_.insert({std::string{sink_key}, this->sink_buf_map_.at(sink_key)});
    }
}

void sink_registry::disable_sink(const std::string_view sink_key) {
    if (this->check_sink_install(sink_key)) {
        this->sink_map_.erase(sink_key);
    }
}

void sink_registry::wipe_sink(const std::string_view sink_key) {
    this->disable_sink(sink_key);

    if (this->check_sink_buf(sink_key)) {
        this->sink_buf_map_.erase(sink_key);
    }
}

// default logging managed by registry
void sink_registry::reg_default(const std::string_view log_filename) {
    // reg default sink if not regged
    if (!this->check_sink_reg(console_sink_key)) {
        this->reg_sink(console_sink_key, []() {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::info);
            return console_sink;
        });
    }

    if (!this->check_sink_reg(file_sink_key)) {
        const std::string log_filename_curr{log_filename};
        this->reg_sink(file_sink_key, [log_filename_curr]() -> spdlog::sink_ptr {
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

void sink_registry::upkeep_default(const log_type_t log_type) {
    if ((log_type == log_type_t::console_only) || (log_type == log_type_t::console_file)) {
        this->enable_sink(console_sink_key);
    } else {
        this->disable_sink(console_sink_key);
    }
    if ((log_type == log_type_t::file_only) || (log_type == log_type_t::console_file)) {
        this->enable_sink(file_sink_key);
    } else {
        this->disable_sink(file_sink_key);
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
            if (sink_registry_.check_sink_buf(sink_key)) {
                auto sink_p = sink_registry_.get_buf_sink(sink_key);
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

std::unique_ptr<spdlog::formatter> group_color_formatter::clone() const { return this->exact_clone(); }

void group_color_formatter::format(const spdlog::details::log_msg &msg, spdlog::memory_buf_t &dest) {
    const auto payload = msg.payload;
    // scan and fill in the colored indexes
    size_t group_count = 0;
    size_t in_group = false;
    bool success = true;
    size_t beg = 0, end = 0;

    // iterate and mark
    for (auto [id, ch] : ranges::subrange(payload.begin(), payload.end()) | ranges::views::enumerate) {
        if (ch == '[') {
            if (in_group) {
                success = false;
                break;
            } else {
                in_group = true;
                group_count += 1;
                if (group_count == group_id) {
                    beg = id + 1;
                }
            }
        } else if (ch == ']') {
            if (in_group) {
                in_group = false;
                if (group_count == group_id) {
                    end = id;
                    break;
                }
            } else {
                success = false;
                break;
            }
        }
    }
    if (success) {
        msg.color_range_start = beg;
        msg.color_range_end = end;
    }

    // append to dest
    spdlog::details::fmt_helper::append_string_view(payload, dest);
    spdlog::details::fmt_helper::append_string_view(eol_, dest);
}

std::unique_ptr<group_color_formatter> group_color_formatter::exact_clone() const {
    return std::make_unique<group_color_formatter>(this->group_id);
}

} // namespace formatter

} // namespace libtcomplex::log