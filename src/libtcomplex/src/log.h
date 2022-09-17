#ifndef LIBTCOMPLEX_LOG_H
#define LIBTCOMPLEX_LOG_H

#include <memory>
#include <spdlog/spdlog.h>
#include <string_view>

namespace libtcomplex::log {
constexpr size_t ASYNC_QUEUE_SIZE = 8192;
constexpr size_t ASYNC_THREAD_COUNT = 1;
constexpr std::string_view ROOT_LOGGER_NAME = "root";

enum struct log_type_t {
    disabled,
    console_only,
    file_only,
    console_file,
};

log_type_t &ref_log_type();
void reset_logging();
std::shared_ptr<spdlog::logger> get_logger(const std::string_view name);
} // namespace libtcomplex::log

#endif
