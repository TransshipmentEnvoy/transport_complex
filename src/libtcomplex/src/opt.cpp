#include "opt.h"

namespace libtcomplex::opt {
const cfg_t *registry_t::select(std::string_view key) const {
    if (!this->cfg_store.contains(std::string{key})) {
        return nullptr;
    } else {
        return this->cfg_store.at(std::string{key}).get();
    }
}

void registry_t::store(std::string_view key, std::unique_ptr<cfg_t> cfg) {
    cfg_store[std::string{key}] = std::move(cfg);
}

} // namespace libtcomplex::opt
