#include "cosche/scheduler.hpp"

#include "cosche/future_task_pair.hpp"
#include "cosche/context_switch.hpp"
#include "cosche/coroutine.hpp"
#include "cosche/future.hpp"
#include "cosche/task.hpp"
#include "cosche/node.hpp"

#include <iostream>

namespace cosche
{

Scheduler::Scheduler() : _isRunning{false} 
{
    mmxFpuSave(&MMX_FPU_STATE);
}

void Scheduler::attach(TaskNode& lhs,
                       TaskNode& rhs)
{
    _taskGraph.attach(lhs,
                      rhs);

    releaseContext(lhs);
}

void Scheduler::attachBatch(TaskNode& taskNode,
                            const std::vector<TaskNode*>& dependees)
{
    _taskGraph.attachBatch(taskNode, dependees);

    releaseContext(taskNode);
}

void Scheduler::detach(TaskNode& lhs,
                       TaskNode& rhs)
{
    _taskGraph.detach(lhs,
                      rhs);
}

void Scheduler::run()
{
    _isRunning = true;

    while (!_taskGraph.empty() || hasFutures())
    {
        auto nextTask = _taskGraph.top().value;

        task::Abstract::TO_RUN = nextTask;
        
        contextSwitch(&(Scheduler::COROUTINE), 
                      &(nextTask->_coroutine));
    }

    _isRunning = false;
}

void Scheduler::pop()
{
    _taskGraph.pop();
}

bool Scheduler::hasFutures()
{
    bool returnValue = !_futuresTaskPairs.empty();

    for (auto&& futureTaskPair : _futuresTaskPairs)
    {
        const auto& [futurePtr, taskNodePtr] = futureTaskPair;

        auto&& taskNode = *taskNodePtr;

        if (futurePtr->ready())
        {
            detach(taskNode,
                   taskNode);

            futureTaskPair = _futuresTaskPairs.back();

            _futuresTaskPairs.pop_back();
        }
    }

    return returnValue;
}

void Scheduler::releaseContext(TaskNode& taskNode)
{
    if (COSCHE_LIKELY(_isRunning))
    {
        auto&& task = *(taskNode.value);

        contextSwitch(&(task._coroutine),
                      &(Scheduler::COROUTINE));
    }
}

thread_local Coroutine Scheduler::COROUTINE;

} // namespace cosche
