#include "cosche/coroutine.hpp"

#include "cosche/register.hpp"
#include "cosche/task.hpp"

namespace cosche
{

Coroutine::Coroutine()
{
    registers[RegisterMap::RDX_RETURN_ADDRESS ] = reinterpret_cast<Register>(&task::Abstract::entryPoint );
    registers[RegisterMap::RCX_STACK_POINTER  ] = reinterpret_cast<Register>(stack.base()                );
    registers[RegisterMap::MMX_FPU_STATE      ] = reinterpret_cast<Register>(MMX_FPU_STATE               );
}

} // namespace cosche