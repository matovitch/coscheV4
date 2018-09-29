#pragma once

#include "likelyhood.hpp"
#include "buffer.hpp"

#include <functional>
#include <cstdint>
#include <memory>
#include <vector>
#include <array>

namespace cosche
{

namespace buffer
{

template <class Buffer>
class TSupervisor
{

public:

    TSupervisor()
    {
        _buffers.emplace_back(std::make_unique<Buffer>());
    }

    void* allocateBucket()
    {
        void* const bucketPtr = _buffers.back()->allocateBucket();

        if (COSCHE_UNLIKELY(bucketPtr == nullptr))
        {
            _buffers.emplace_back(_buffers.back()->makeNext());
            return _buffers.back()->allocateBucket();
        }

        return bucketPtr;
    }

    void clean(const std::function<void(void*)>& destructor)
    {
        for (auto&& buffer : _buffers)
        {
            buffer->clean(destructor);
        }
    }

private:

    std::vector<std::unique_ptr<Buffer>> _buffers;
};

namespace supervisor
{

template <class Type>
using TMakeFromType = TSupervisor<buffer::TMakeFromType<Type>>;

} // namespace supervisor

} // namespace buffer

} // namespace cosche
