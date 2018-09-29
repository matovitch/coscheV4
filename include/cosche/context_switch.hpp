#pragma once

#include "cosche/coroutine.hpp"
#include "cosche/register.hpp"

extern "C" void* coscheContextSwitch(cosche::Coroutine* srce,
                                     cosche::Coroutine* dest);

extern "C" void coscheMmxFpuSave(cosche::Register* mmxFpuState);

namespace cosche
{

void* contextSwitch(cosche::Coroutine* srce,
                    cosche::Coroutine* dest);

void mmxFpuSave(cosche::Register* mmxFpuState);

} // namespace cosche
