#ifndef LIBTCOMPLEX_LOG_DEF_H
#define LIBTCOMPLEX_LOG_DEF_H

#include <memory>
#include <string_view>

namespace spdlog {
class logger;
}; // namespace spdlog

namespace libtcomplex::interface::log {

bool check_init();

struct LogCtx {
public:
    LogCtx(const std::string_view key = "python");
    void set_level();
    void set_format();
    void log(const int lvl, const char *msg, double created, const char *filename, const char *funcname, int lineno);

private:
    std::string key;
    std::shared_ptr<spdlog::logger> logger;
};

} // namespace libtcomplex::interface::log

#endif