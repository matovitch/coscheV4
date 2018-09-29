#pragma once

#include "robin/buffer_view.hpp"

#include <memory>

namespace robin
{

namespace buffer
{

template <class AbstractTraits>
class TAbstract
{
    using Heap = typename AbstractTraits::Heap;
    using View = typename AbstractTraits::View;
    
public:

    virtual                 View  makeView()       = 0;
    virtual std::unique_ptr<Heap> makeNext() const = 0;

    virtual ~TAbstract() {}
};

template <class>
class THeap;

namespace heap
{

template <std::size_t, std::size_t>
struct TTraits;

} // namespace heap

namespace abstract
{

template <std::size_t SIZEOF,
          std::size_t ALIGNOF>
struct TTraits
{    
    using HeapTraits = heap::TTraits<SIZEOF, ALIGNOF>;
    
    using Heap = THeap<HeapTraits>;
    
    using View = TView<SIZEOF>;
};

template <class Type>
using TMakeTraitsFromType = TTraits<sizeof  (Type),
                                    alignof (Type)>;

} // namespace abstract

} // namespace buffer

} // namespace robin
