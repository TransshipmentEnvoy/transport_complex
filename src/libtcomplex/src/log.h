#ifndef LIBTCOMPLEX_LOG_H
#define LIBTCOMPLEX_LOG_H

#include <memory>
#include <optional>
#include <string_view>

#include <spdlog/spdlog.h>

#include <libtcomplex/log_def.h>

namespace libtcomplex::log {
enum struct log_type_t;                          // forward declare
void reset_logging(const log_param_t log_param); // forward declare

constexpr size_t ASYNC_QUEUE_SIZE = 8192;
constexpr size_t ASYNC_THREAD_COUNT = 2;
constexpr std::string_view ROOT_LOGGER_NAME = "root";

std::shared_ptr<spdlog::logger> get_logger(const std::string_view name);
} // namespace libtcomplex::log

#endif
