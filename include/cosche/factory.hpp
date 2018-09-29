#pragma once

#include "buffer_supervisor.hpp"
#include "singleton.hpp"

#include <utility>
#include <vector>

namespace cosche
{

template <class Type>
class TFactory
{
    using Allocator = buffer::supervisor::TMakeFromType<Type>;

public:

    template <class... ARGS>
    Type& make(ARGS&&... args)
    {
        void* ptr;

        if (!_recycleds.empty())
        {
            ptr = reinterpret_cast<void*>(_recycleds.back());
            _recycleds.pop_back();
            destroy(ptr);
        }
        else
        {
            ptr = _allocator.allocateBucket();
        }

        new(ptr) Type(std::forward<ARGS>(args)...);

        return *(reinterpret_cast<Type*>(ptr));
    }

    void recycle(Type* const typePtr)
    {
        _recycleds.push_back(typePtr);
    }

    ~TFactory()
    {
        _allocator.clean(&destroy);
    }

private:

    static void destroy(void* ptr)
    {
        reinterpret_cast<Type*>(ptr)->~Type();
    }

    std::vector<Type*> _recycleds;

    static Allocator   _allocator;
};

template <class Type>
typename TFactory<Type>::Allocator TFactory<Type>::_allocator;

} // namespace cosche
