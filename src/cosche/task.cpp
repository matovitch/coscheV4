#include "cosche/task.hpp"

#include "cosche/context_switch.hpp"
#include "cosche/scheduler.hpp"

namespace cosche
{

namespace task
{

void Abstract::entryPoint()
{
    TO_RUN->run();

    TO_RUN->_scheduler.pop();

    contextSwitch(&(TO_RUN->_coroutine),
                  &(Scheduler::COROUTINE));
}

Abstract::Abstract(Scheduler& scheduler) :
    _scheduler {scheduler}
{}

Abstract::~Abstract() {}

thread_local Abstract* Abstract::TO_RUN;

} // namespace task

} // namepsace cosche