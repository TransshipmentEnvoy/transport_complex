#ifndef LIBTCOMPLEX_LOG_H
#define LIBTCOMPLEX_LOG_H

#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include <spdlog/formatter.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/spdlog.h>

#include <tsl/robin_map.h>

#include <libtcomplex/log_def.h>

namespace libtcomplex::log {
enum struct log_type_t;                          // forward declare
void reset_logging(const log_param_t log_param); // forward declare

constexpr size_t ASYNC_QUEUE_SIZE = 8192;
constexpr size_t ASYNC_THREAD_COUNT = 2;
constexpr std::string_view ROOT_LOGGER_NAME = "root";

namespace formatter {
class condition_pattern_formatter;
}

constexpr std::string_view console_sink_key = "console";
constexpr std::string_view file_sink_key = "file";

class sink_registry final {
public:
    using sink_map = tsl::robin_map<std::string, spdlog::sink_ptr, std::hash<std::string_view>, std::equal_to<>>;
    using sink_condition_formatter_map =
        tsl::robin_map<std::string, std::unique_ptr<formatter::condition_pattern_formatter>,
                       std::hash<std::string_view>, std::equal_to<>>;
    using sink_init_fn = std::function<spdlog::sink_ptr()>;
    using sink_init_fn_map = tsl::robin_map<std::string, sink_init_fn, std::hash<std::string_view>, std::equal_to<>>;

public:
    explicit sink_registry();

    inline bool check_sink_reg(const std::string_view sink_key) const {
        return this->sink_init_fn_map_.contains(sink_key);
    }
    inline void reg_sink(const std::string_view sink_key, sink_init_fn &&init_fn) {
        this->sink_init_fn_map_.insert({std::string{sink_key}, std::move(init_fn)});
    }
    inline void unreg_sink(const std::string_view sink_key) { this->sink_init_fn_map_.erase(sink_key); }

    inline bool check_sink_install(const std::string_view sink_key) const { return this->sink_map_.contains(sink_key); }
    inline void install_sink(const std::string_view sink_key, const spdlog::sink_ptr sptr) {
        this->sink_map_.insert({std::string{sink_key}, sptr});
    }
    inline void uninstall_sink(const std::string_view sink_key) { this->sink_map_.erase(sink_key); }
    inline spdlog::sink_ptr get_sink(const std::string_view sink_key) {
        return (check_sink_install(sink_key)) ? this->sink_map_.at(sink_key) : nullptr;
    }
    inline const sink_map &sink_items() const { return sink_map_; }

    inline bool check_sink_formatter(const std::string_view sink_key) const {
        return this->sink_formatter_map_.contains(sink_key);
    }
    void install_sink_formatter(const std::string_view sink_key,
                                std::unique_ptr<formatter::condition_pattern_formatter> f) {
        this->sink_formatter_map_.insert({std::string{sink_key}, std::move(f)});
    }
    // no uninstall_sink_formatter! as formatter handling is non trivial
    formatter::condition_pattern_formatter *get_formatter(const std::string_view sink_key) {
        return this->sink_formatter_map_.at(sink_key).get();
    }

    // default logging
    void reg_default(const log_type_t log_type, const std::string_view log_filename);
    void upkeep();

private:
    std::unique_ptr<formatter::condition_pattern_formatter> default_formatter;

    sink_map sink_map_;                               // store active sinks
    sink_condition_formatter_map sink_formatter_map_; // store active sink formatters
    sink_init_fn_map sink_init_fn_map_;               // store logic used in reset_logging
};

using sink_formatter_map =
    tsl::robin_map<std::string, std::unique_ptr<spdlog::formatter>, std::hash<std::string_view>, std::equal_to<>>;

inline sink_registry &ref_sink_registry() {
    static sink_registry sink_registry_;
    return sink_registry_;
}

std::shared_ptr<spdlog::logger> get_logger(const std::string_view name);
std::shared_ptr<spdlog::logger> get_logger(const std::string_view name, sink_formatter_map fmap);

// formater condtions on registered logger name
namespace formatter {
class condition_pattern_formatter final : public spdlog::formatter {
    using formatter_map =
        tsl::robin_map<std::string, std::unique_ptr<spdlog::formatter>, std::hash<std::string_view>, std::equal_to<>>;

public:
    explicit condition_pattern_formatter(std::unique_ptr<spdlog::formatter> f) : formatter_default(std::move(f)) {}
    explicit condition_pattern_formatter(std::unique_ptr<spdlog::formatter> f, formatter_map &&store)
        : formatter_default(std::move(f)), formatter_map_(std::move(store)){};

    condition_pattern_formatter(const condition_pattern_formatter &other) = delete;
    condition_pattern_formatter &operator=(const condition_pattern_formatter &other) = delete;

    std::unique_ptr<formatter> clone() const override;
    void format(const spdlog::details::log_msg &msg, spdlog::memory_buf_t &dest) override;

    void add_condition(const std::string_view name, std::unique_ptr<spdlog::formatter> f);
    std::unique_ptr<condition_pattern_formatter> exact_clone() const;

private:
    formatter_map formatter_map_;
    std::unique_ptr<spdlog::formatter> formatter_default;
};

class group_color_formatter final : public spdlog::formatter {
public:
    explicit group_color_formatter(const size_t gid) : group_id(gid){};

    group_color_formatter(const group_color_formatter &other) = delete;
    group_color_formatter &operator=(const group_color_formatter &other) = delete;

    std::unique_ptr<formatter> clone() const override;
    void format(const spdlog::details::log_msg &msg, spdlog::memory_buf_t &dest) override;
    std::unique_ptr<group_color_formatter> exact_clone() const;

private:
    size_t group_id;
};

} // namespace formatter

} // namespace libtcomplex::log

#endif
