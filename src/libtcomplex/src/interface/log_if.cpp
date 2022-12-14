#include "../log.h"
#include <chrono>
#include <libtcomplex/interface/log_if.h>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>

namespace libtcomplex::interface::log {

using namespace libtcomplex::log;

bool check_init() {
    auto root = spdlog::get(std::string{ROOT_LOGGER_NAME});
    return (root != nullptr);
};

LogCtx::LogCtx(const std::string key) : key{key} {
    SPDLOG_INFO("x");
    auto f_console = std::make_unique<formatter::group_color_formatter>(3);
    auto f_file = std::make_unique<spdlog::pattern_formatter>("%v");
    sink_formatter_map fmap;
    fmap.insert({std::string{console_sink_key}, std::move(f_console)});
    fmap.insert({std::string{file_sink_key}, std::move(f_file)});
    this->logger = get_logger(this->key, std::move(fmap));
    SPDLOG_INFO("y");
}

spdlog::level::level_enum get_level_from_py(const int lvl) {
    spdlog::level::level_enum res;
    switch (lvl) {
    case 50:
        res = spdlog::level::critical;
        break;
    case 40:
        res = spdlog::level::err;
        break;
    case 30:
        res = spdlog::level::warn;
        break;
    case 20:
        res = spdlog::level::info;
        break;
    case 10:
        res = spdlog::level::debug;
        break;
    default:
        res = spdlog::level::trace;
        break;
    }
    return res;
}

void LogCtx::log(const int lvl, const char *msg, double created, const char *filename, const char *funcname,
                 const int lineno) {
    using spdlog::level::to_string_view;

    const spdlog::level::level_enum loglvl = get_level_from_py(lvl);

    using time_point = std::chrono::system_clock::time_point;
    const std::chrono::duration<double> _ct{created};
    const auto _ct_ns = std::chrono::duration_cast<time_point::duration>(_ct);
    const time_point ct{_ct_ns};
    const auto second_part = std::chrono::time_point_cast<std::chrono::seconds>(ct);
    const auto fraction = ct - second_part;
    const auto millisecond_part = std::chrono::duration_cast<std::chrono::milliseconds>(fraction);

    this->logger->log(loglvl,                                                  //
                      "[{:%Y-%m-%d %H:%M:%S}.{:0>3}] [{}] [{}] [{}:{}:{}] {}", //
                      ct, millisecond_part.count(), this->key, to_string_view(loglvl), filename, lineno, funcname, msg);
}

} // namespace libtcomplex::interface::log
