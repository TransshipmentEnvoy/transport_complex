#ifndef LIBTCOMPLEX_CKPT_H
#define LIBTCOMPLEX_CKPT_H

#include "opt.h"
#include <filesystem>

namespace libtcomplex::ckpt {
struct cfg_t : libtcomplex::opt::cfg_t {
    std::string sess_id;
    std::filesystem::path ckpt_path;
    std::filesystem::path log_file;
};
} // namespace libtcomplex::ckpt

#endif