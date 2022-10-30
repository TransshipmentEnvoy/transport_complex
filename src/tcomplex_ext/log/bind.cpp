#include <nanobind/nanobind.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/stl/optional.h>

#include <libtcomplex/interface/log_if.h>

using namespace libtcomplex::interface::log;
namespace nb = nanobind;

NB_MODULE(_log, m) {
    nb::enum_<log_type_t>(m, "log_type_t")
        .value("disabled", log_type_t::disabled)
        .value("console_only", log_type_t::console_only)
        .value("file_only", log_type_t::file_only)
        .value("console_file", log_type_t::console_file);

    m.def("reset_logging", reset_logging, nb::arg("log_type_in") = nb::none());
    
    m.def("check_init", check_init);
    
    nb::class_<LogCtx>(m, "LogCtx")
        .def(nb::init<>())
        .def(nb::init<const std::string_view>())
        .def("log", &LogCtx::log);
}