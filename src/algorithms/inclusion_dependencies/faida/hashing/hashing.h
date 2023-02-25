#pragma once

#include "immintrin.h"

namespace hashing {

inline size_t rotl(size_t hash, int num_bits) {
    return _lrotl(hash, num_bits);
}

} //namespace hashing
