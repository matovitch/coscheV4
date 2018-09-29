#pragma once

#include "future.hpp"
#include "task.hpp"
#include "node.hpp"

namespace cosche
{

struct FutureTaskPair
{
    FutureTaskPair(future::Abstract       * const futurePtr,
                   TNode<task::Abstract*> * const taskNodePtr);

    future::Abstract       * _futurePtr;
    TNode<task::Abstract*> * _taskNodePtr;
};

} // namespace cosche