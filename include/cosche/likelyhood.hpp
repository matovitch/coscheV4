#pragma once

#if defined __GNUC__ || \
    defined __llvm__
    #define COSCHE_LIKELY(x) __builtin_expect ((x), 1)
    #define COSCHE_UNLIKELY(x) __builtin_expect ((x), 0)
#else
    #define COSCHE_LIKELY(x) (x)
    #define COSCHE_UNLIKELY(x) (x)
#endif