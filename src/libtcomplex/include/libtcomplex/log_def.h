#ifndef LIBTCOMPLEX_LOG_DEF_H
#define LIBTCOMPLEX_LOG_DEF_H

#include <optional>

namespace libtcomplex::log {
enum struct log_type_t {
    disabled,
    console_only,
    file_only,
    console_file,
};

void reset_logging(const std::optional<log_type_t> log_type_in = std::nullopt); // impl in log.cpp

} // namespace libtcomplex::log

#endif