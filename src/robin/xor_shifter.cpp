#include "robin/xor_shifter.hpp"

#include <cstdint>

namespace robin
{

uint64_t XorShifter::operator()()
{
    _state_0 ^= (_state_0 << 5);
    _state_0 ^= (_state_0 >> 15);
    _state_0 ^= (_state_0 << 27);

    _state_1 += BASE_STATE_1;

    return _state_0 + (_state_1 ^ (_state_1 >> 27));
}

} // namespace robin