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

Scheduler::Scheduler()
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

void Scheduler::attach(TaskNode& dependee)
{
    if (COSCHE_UNLIKELY(_me == nullptr))
    {
        return;
    }

    attach(*_me, dependee);
}

void Scheduler::detach(TaskNode& lhs,
                       TaskNode& rhs)
{
    _taskGraph.detach(lhs,
                      rhs);
}

void Scheduler::detach(TaskNode& depender)
{
    if (COSCHE_UNLIKELY(_me == nullptr))
    {
        return;
    }

    _taskGraph.detach(depender, *_me);
}

void Scheduler::attachBatch(TaskNode& taskNode,
                            const std::vector<TaskNode*>& dependees)
{
    _taskGraph.attachBatch(taskNode, dependees);

    releaseContext(taskNode);
}

void Scheduler::attachBatch(const std::vector<TaskNode*>& dependees)
{
    if (COSCHE_UNLIKELY(_me == nullptr))
    {
        return;
    }

    attachBatch(*_me, dependees);
}

void Scheduler::run()
{
    while (!_taskGraph.empty() || hasFutures())
    {
        _me = &(_taskGraph.top());

        contextSwitch(&(Scheduler::_coroutine),
                      &(_me->value->_coroutine));
    }

    _me = nullptr;
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
    if (COSCHE_UNLIKELY(_me == nullptr))
    {
        return;
    }

    auto&& task = *(taskNode.value);

    contextSwitch(&(task._coroutine),
                  &(Scheduler::_coroutine));
}

void Scheduler::taskEntryPoint()
{
    auto* const taskPtr = _me->value;

    taskPtr->run();
    taskPtr->_scheduler._taskGraph.pop();

    contextSwitch(&(taskPtr->_coroutine),
                  &(         _coroutine));
}

thread_local Coroutine Scheduler::_coroutine;

thread_local Scheduler::TaskNode* Scheduler::_me = nullptr;

} // namespace cosche
