#ifndef LIBTCOMPLEX_LOG_H
#define LIBTCOMPLEX_LOG_H

#include <memory>
#include <spdlog/spdlog.h>
#include <string_view>

namespace libtcomplex::log {
void setup_logging();
std::shared_ptr<spdlog::logger> get_logger(const std::string_view name);
} // namespace libtcomplex::log

#endif
