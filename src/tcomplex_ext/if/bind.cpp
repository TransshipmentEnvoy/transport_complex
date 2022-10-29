#include <nanobind/nanobind.h>

#include <libtcomplex/lib.h>

NB_MODULE(_if, m) {
    m.def("run", libtcomplex::run);

    // init fn
    m.def("init", libtcomplex::init);

    // deinit fn
    m.def("deinit", libtcomplex::deinit);
}