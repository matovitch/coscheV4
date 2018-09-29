#pragma once

#include "likelyhood.hpp"

#include <type_traits>
#include <functional>
#include <cstdint>
#include <memory>

namespace cosche
{

template <class BufferTraits>
class TBuffer
{
    using Bucket = typename BufferTraits::Bucket;
    using Buffer = typename BufferTraits::Buffer;

public:

    TBuffer(const std::size_t size = 1) :
        _head{new Bucket[size]},
        _tail{_head + size},
        _data{_head}
    {}

    void* allocateBucket()
    {
        if (COSCHE_UNLIKELY(_tail == _head))
        {
            return nullptr;
        }

        void* const typePtr = reinterpret_cast<void*>(_head);

        _head++;

        return typePtr;
    }

    std::unique_ptr<Buffer> makeNext() const
    {
        return std::make_unique<Buffer>((_tail - _data) << 1);
    }

    void clean(const std::function<void(void*)>& destructor)
    {
        Bucket* head = _data;

        while (head != _head)
        {
            destructor(reinterpret_cast<void*>(head));
            head++;
        }
    }

    ~TBuffer()
    {
        delete[] _data;
    }

private:

          Bucket*       _head;
    const Bucket* const _tail;
          Bucket* const _data;
};

namespace buffer
{

template <std::size_t SIZEOF,
          std::size_t ALIGNOF>
struct TTraits
{
    using Bucket = std::aligned_storage_t <SIZEOF, ALIGNOF>;

    using Traits = TTraits<SIZEOF, ALIGNOF>;
    using Buffer = TBuffer<Traits>;
};

template <class Type>
using TMakeTraitsFromType = TTraits<sizeof  (Type),
                                    alignof (Type)>;

template <class Type>
using TMakeFromType = TBuffer<TMakeTraitsFromType<Type>>;

} // namespace buffer

} // namespace cosche
