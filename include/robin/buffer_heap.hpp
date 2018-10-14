#pragma once

#include "robin/buffer_abstract.hpp"
#include "robin/buffer_view.hpp"

#include <type_traits>
#include <cstdint>
#include <memory>

namespace robin
{

namespace buffer
{

template <class HeapTraits>
class THeap : public TAbstract<typename HeapTraits::AbstractTraits>
{
    using Bucket = typename HeapTraits::Bucket;
    using Heap   = typename HeapTraits::Heap;
    using View   = typename HeapTraits::View;

public:

    THeap(const std::size_t size) :
        _data{new Bucket[size]},
        _size{size}
    {}

    View makeView() override
    {
        return {reinterpret_cast<uint8_t*>(_data), _size};
    }

    std::unique_ptr<Heap> makeNext() const override
    {
        return std::make_unique<Heap>(((_size - 1) << 1) + 1);
    }

    std::unique_ptr<Heap> makePrev() const override
    {
        return std::make_unique<Heap>(((_size - 1) >> 2) + 1);
    }

    ~THeap() override
    {
        delete[] _data;
    }

private:

    Bucket* const    _data;
    const std::size_t _size;
};

namespace heap
{

template <std::size_t SIZEOF,
          std::size_t ALIGNOF>
struct TTraits
{
    using AbstractTraits =      abstract::TTraits <SIZEOF, ALIGNOF>;
    using Bucket         = std::aligned_storage_t <SIZEOF, ALIGNOF>;

    using Heap = THeap<TTraits<SIZEOF, ALIGNOF>>;

    using View = TView<SIZEOF>;
};

template <class Type>
using TMakeTraitsFromType = TTraits<sizeof  (Type),
                                    alignof (Type)>;
} // namespace heap

} // namespace buffer

} // namespace robin
