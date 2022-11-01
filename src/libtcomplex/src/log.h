#ifndef LIBTCOMPLEX_LOG_H
#define LIBTCOMPLEX_LOG_H

#include <memory>
#include <optional>
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

std::shared_ptr<spdlog::logger> get_logger(const std::string_view name);
std::shared_ptr<spdlog::logger> get_logger(const std::string_view name, const std::string_view pattern);

// formater condtions on registered logger name
namespace formatter {
class condition_pattern_formater final : public spdlog::formatter {
    using formatter_map =
        tsl::robin_map<std::string, std::unique_ptr<spdlog::formatter>, std::hash<std::string_view>, std::equal_to<>>;

public:
    explicit condition_pattern_formater(std::unique_ptr<spdlog::formatter> f) : formatter_default(std::move(f)) {}
    explicit condition_pattern_formater(std::unique_ptr<spdlog::formatter> f, formatter_map &&store)
        : formatter_default(std::move(f)), formatter_map_(std::move(store)){};

    condition_pattern_formater(const condition_pattern_formater &other) = delete;
    condition_pattern_formater &operator=(const condition_pattern_formater &other) = delete;

    std::unique_ptr<formatter> clone() const override;
    void format(const spdlog::details::log_msg &msg, spdlog::memory_buf_t &dest) override;

    void add_condition(const std::string_view name, std::unique_ptr<spdlog::formatter> f);

private:
    formatter_map formatter_map_;
    std::unique_ptr<spdlog::formatter> formatter_default;
};

} // namespace formatter

} // namespace libtcomplex::log

#endif
