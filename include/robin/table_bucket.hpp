#pragma once

#include "robin/default_constructible.hpp"

#include <cstdint>

namespace robin
{

namespace table
{

template <typename Type>
class TBucket
{

public:

    enum
    {
        EMPTY  = 0,
        FILLED = 1
    };

    TBucket() : _dib{EMPTY} {}

    void markEmpty () { _dib = EMPTY ; }
    void markFilled() { _dib = FILLED; }

    bool isEmpty () const { return _dib == EMPTY; }
    bool isFilled() const { return _dib != EMPTY; }

    template<class... Args>
    void fill(uint8_t dib, Args&&... args)
    {
        _dib = dib;
        _val.construct(args...);
    }

    uint8_t dib() const { return _dib; }

    ~TBucket()
    {
        if (!isEmpty())
        {
            _val.destroy();
        }
    }

          Type&  value()       &  {              return _val.value(); }
          Type&& value()       && { markEmpty(); return _val.value(); }
    const Type&  value() const &  {              return _val.value(); }

private:

    uint8_t                    _dib;
    DefaultConstructible<Type> _val;
};

} // namespace table

} // namespace robin
