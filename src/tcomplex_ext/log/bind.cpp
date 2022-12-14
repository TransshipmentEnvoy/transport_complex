#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>

#include <libtcomplex/interface/log_if.h>

using namespace libtcomplex::interface::log;
namespace nb = nanobind;

NB_MODULE(_log, m) {
    nb::enum_<log_type_t>(m, "log_type_t")
        .value("disabled", log_type_t::disabled)
        .value("console_only", log_type_t::console_only)
        .value("file_only", log_type_t::file_only)
        .value("console_file", log_type_t::console_file);

    nb::class_<log_param_t>(m, "log_param_t")
        .def(nb::init<std::optional<log_type_t>, std::optional<std::string>>(), //
             nb::arg("log_type") = nb::none(),
             nb::arg("log_filename") = nb::none())         //
        .def_readwrite("log_type", &log_param_t::log_type) //
        .def_readwrite("log_filename", &log_param_t::log_filename);

    m.def("reset_logging", reset_logging, nb::arg("log_param") = log_param_default);
    m.def("flush_logging", flush_logging);

    m.def("check_init", check_init);

    nb::class_<LogCtx>(m, "LogCtx")
        .def(nb::init<>())                  //
        .def(nb::init<const std::string>()) //
        .def("log", &LogCtx::log);
}