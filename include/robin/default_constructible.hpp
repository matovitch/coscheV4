#pragma once

#include <cstdint>
#include <utility>

namespace robin
{

template <class Type>
union DefaultConstructible
{

public:

    DefaultConstructible() : _dummy{} {}
    
    template<class... Args>
    void construct(Args&&... args)
    {
        new (static_cast<void*>(&_value)) Type(std::forward<Args>(args)...);
    }

    void destroy()
    {
        _value.~Type();
    }

          Type&  value()       &  { return           _value  ; }
          Type&& value()       && { return std::move(_value) ; }
    const Type&  value() const &  { return           _value  ; }

private:

    uint8_t _dummy;
    Type    _value;
};

} // namespace robin
