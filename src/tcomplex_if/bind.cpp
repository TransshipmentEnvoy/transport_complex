#include <nanobind/nanobind.h>

#include <libtcomplex/lib.h>

NB_MODULE(_if, m) {
    m.def("run", libtcomplex::run);
}