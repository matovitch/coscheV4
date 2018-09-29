#include "cosche/context_switch.hpp"

namespace cosche
{

void* contextSwitch(cosche::Coroutine* srce,
                    cosche::Coroutine* dest)
{
    return coscheContextSwitch(srce, dest);
}

void mmxFpuSave(cosche::Register* mmxFpuState)
{
    coscheMmxFpuSave(mmxFpuState);
}

} // namespace cosche
