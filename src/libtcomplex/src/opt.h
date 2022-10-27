#ifndef LIBTCOMPLEX_OPT_H
#define LIBTCOMPLEX_OPT_H

#include <memory>
#include <string>
#include <tsl/robin_map.h>

namespace libtcomplex::opt {
struct cfg_t {};

struct registry_t {
public:
    const cfg_t *select(std::string_view key) const;
    void store(std::string_view key, std::unique_ptr<cfg_t> cfg);

private:
    tsl::robin_map<std::string, std::unique_ptr<cfg_t>> cfg_store;
};

// global registry: for game startup/shutdown/logging/search_path
registry_t &ref_global_registry();
} // namespace libtcomplex::opt

#endif