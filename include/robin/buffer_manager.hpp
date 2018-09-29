#pragma once

#include "robin/buffer_abstract.hpp"
#include "robin/buffer_stack.hpp"
#include "robin/buffer_heap.hpp"
#include "robin/buffer_view.hpp"

namespace robin
{

namespace buffer
{

template <class ManagerTraits>
class TManager
{
    static constexpr std::size_t SIZEOF  = ManagerTraits::SIZEOF;
                                     
    using Abstract = typename ManagerTraits::Abstract;
    using Stack    = typename ManagerTraits::Stack;
    using Heap     = typename ManagerTraits::Heap;
    using View     = typename ManagerTraits::View;

public:

    TManager() : _bufferPtr{&_stack} {}
    
    View makeView()
    {
        return  _bufferPtr->makeView();   
    }
    
    View makeNextView()
    {
        _nextPtr = std::move(_bufferPtr->makeNext());
        return _nextPtr->makeView();
    }
    
    void swapBuffers()
    {
        _heapPtr = std::move(_nextPtr);
        _bufferPtr = _heapPtr.get();
    }
    
private:

    Abstract*             _bufferPtr;
    Stack                 _stack;
    std::unique_ptr<Heap> _heapPtr;
    std::unique_ptr<Heap> _nextPtr;
};

namespace manager
{

template <std::size_t MANAGER_SIZE,
          std::size_t MANAGER_SIZEOF,
          std::size_t MANAGER_ALIGNOF>
struct TTraits
{
    static constexpr std::size_t SIZE    = MANAGER_SIZE;
    static constexpr std::size_t SIZEOF  = MANAGER_SIZEOF;
    static constexpr std::size_t ALIGNOF = MANAGER_ALIGNOF; 

    using StackTraits    = stack::TTraits    <SIZE,
                                              SIZEOF,
                                              ALIGNOF>;
                                       
    using HeapTraits     = heap::TTraits     <SIZEOF,
                                              ALIGNOF>;
                                     
    using AbstractTraits = abstract::TTraits <SIZEOF,
                                              ALIGNOF>;
                                     
    using Abstract = TAbstract <AbstractTraits>;
    using Stack    = TStack    <StackTraits>;
    using Heap     = THeap     <HeapTraits>;
    
    using View = TView<SIZEOF>;
};

template <class Type, std::size_t SIZE>
using TMakeTraitsFromType = TTraits<SIZE,
                                    sizeof  (Type),
                                    alignof (Type)>;

template <class Type, std::size_t SIZE>
using TMakeFromType = TManager<TMakeTraitsFromType<Type, SIZE>>;

} // namespace manager

} // namespace buffer

} // namespace robin
