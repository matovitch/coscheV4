#pragma once

#include <type_traits>
#include <cstdint>

namespace cosche
{

template <std::size_t SIZE>
class TStack
{
    // Note, the offset is needed to make room for
    // the return address used upon coroutine exit
    static constexpr std::size_t OFFSET = 0x08;
    static constexpr std::size_t ALIGN  = 0x10;

    static constexpr std::size_t ALIGNED_SIZE = SIZE - SIZE % ALIGN;

public:

    void* base()
    {
        return reinterpret_cast<void*>(&_memory) + ALIGNED_SIZE - OFFSET;
    }

private:

    std::aligned_storage_t<ALIGNED_SIZE, ALIGN> _memory;
};

} // namespace cosche