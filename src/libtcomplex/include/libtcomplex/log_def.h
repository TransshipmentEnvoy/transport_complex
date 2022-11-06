#ifndef LIBTCOMPLEX_LOG_DEF_H
#define LIBTCOMPLEX_LOG_DEF_H

#include <optional>
#include <string>
#include <string_view>

namespace libtcomplex::log {
enum struct log_type_t {
    disabled,
    console_only,
    file_only,
    console_file,
};

struct log_param_t {
    std::optional<log_type_t> log_type;
    std::optional<std::string> log_filename;
};

static const log_param_t log_param_default{std::nullopt, std::nullopt};

void reset_logging(const log_param_t log_param = log_param_default); // impl in log.cpp

} // namespace libtcomplex::log

#endif