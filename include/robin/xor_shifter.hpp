#pragma once

#include <cstdint>

namespace robin
{

class XorShifter
{
    static constexpr uint64_t BASE_STATE_0 = 0xD2D45C2500000000;
    static constexpr uint64_t BASE_STATE_1 = 0x61C8864680B583EB;

public:

    uint64_t operator()();

private:

    uint64_t _state_0 = BASE_STATE_0;
    uint64_t _state_1 = BASE_STATE_1;
};

} // namespace robin