#pragma once

#include "cosche/register.hpp"
#include "cosche/stack.hpp"

namespace cosche
{

struct Coroutine
{
    static constexpr std::size_t STACK_SIZE = 16 * 1024;

    Coroutine();
    
    Register registers[RegisterMap::SIZE];

    TStack<STACK_SIZE> stack;
};

} // namespace cosche