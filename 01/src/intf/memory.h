#pragma once

#include "types.h"

static inline void memset(void* dst, uint8_t value, size_t n) {
    uint8_t* d = (uint8_t*) dst;

    while (n-- > 0) {
        *d++ = value;
    }
}

