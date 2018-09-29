#pragma once

#include <cstdint>

namespace robin
{

namespace buffer
{ 

template <std::size_t VIEW_SIZEOF>
struct TView
{
    static constexpr std::size_t SIZEOF = VIEW_SIZEOF;

    uint8_t* const    data;
    const std::size_t size;
};

} // namespace buffer

} // namespace robin
