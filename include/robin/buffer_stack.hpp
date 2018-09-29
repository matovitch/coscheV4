#pragma once

#include "buffer_abstract.hpp"
#include "buffer_view.hpp"

#include <type_traits>
#include <cstdint>
#include <memory>

namespace robin
{

namespace buffer
{

template <class StackTraits>
class TStack : public TAbstract<typename StackTraits::AbstractTraits>
{
    static constexpr std::size_t SIZE = StackTraits::SIZE;
    
    using Bucket = typename StackTraits::Bucket;
    using Heap   = typename StackTraits::Heap;
    using View   = typename StackTraits::View;

public:

    View makeView() override
    {
        return {reinterpret_cast<uint8_t*>(_array.data()), SIZE + 1};    
    }
    
    std::unique_ptr<Heap> makeNext() const override
    {
        return std::make_unique<Heap>((SIZE << 1) + 1);
    }

private:

    std::array<Bucket, SIZE + 1> _array;
};

namespace stack
{

template <std::size_t STACK_SIZE,
          std::size_t STACK_SIZEOF,
          std::size_t STACK_ALIGNOF>
struct TTraits
{   
    static constexpr std::size_t SIZE    = STACK_SIZE;
    static constexpr std::size_t SIZEOF  = STACK_SIZEOF;
    static constexpr std::size_t ALIGNOF = STACK_ALIGNOF;
    
    using AbstractTraits =      abstract::TTraits <SIZEOF, ALIGNOF>;
    using HeapTraits     =          heap::TTraits <SIZEOF, ALIGNOF>;
    using Bucket         = std::aligned_storage_t <SIZEOF, ALIGNOF>;
    
    using Heap = THeap<HeapTraits>;
    
    using View = TView<SIZEOF>;
};

template <class Type, std::size_t SIZE>
using TMakeTraitsFromType = TTraits<SIZE,
                                    sizeof  (Type),
                                    alignof (Type)>;
} // namespace stack

} // namespace buffer

} // namespace robin
