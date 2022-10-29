#include <nanobind/nanobind.h>
#include <nanobind/stl/string_view.h>

#include <libtcomplex/log_if.h>

using namespace libtcomplex::interface::log;
namespace nb = nanobind;

NB_MODULE(_log, m) {
    m.def("check_init", check_init);
    nb::class_<LogCtx>(m, "LogCtx")
        .def(nb::init<>())
        .def(nb::init<const std::string_view>())
        .def("log", &LogCtx::log);
}