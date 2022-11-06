#ifndef LIBTCOMPLEX_INTERFACE_LOG_IF_H
#define LIBTCOMPLEX_INTERFACE_LOG_IF_H

#include <memory>
#include <string>
#include <string_view>

#include "../log_def.h"

namespace spdlog {
class logger;
}; // namespace spdlog

namespace libtcomplex::interface::log {

using libtcomplex::log::log_param_default;
using libtcomplex::log::log_param_t;
using libtcomplex::log::log_type_t;
using libtcomplex::log::reset_logging;

bool check_init();

struct LogCtx {
public:
    LogCtx(const std::string key = "python");
    void set_level();
    void set_format();
    void log(const int lvl, const char *msg, double created, const char *filename, const char *funcname, int lineno);

private:
    std::string key;
    std::shared_ptr<spdlog::logger> logger;
};

} // namespace libtcomplex::interface::log

#endif