#ifndef LIBTCOMPLEX_LOG_H
#define LIBTCOMPLEX_LOG_H

#include <memory>
#include <optional>
#include <string_view>

#include <spdlog/spdlog.h>

#include <libtcomplex/log_def.h>

namespace libtcomplex::log {
constexpr size_t ASYNC_QUEUE_SIZE = 8192;
constexpr size_t ASYNC_THREAD_COUNT = 1;
constexpr std::string_view ROOT_LOGGER_NAME = "root";

enum struct log_type_t;                                          // forward declare
void reset_logging(const std::optional<log_type_t> log_type_in); // forward declare

std::shared_ptr<spdlog::logger> get_logger(const std::string_view name);
} // namespace libtcomplex::log

#endif
