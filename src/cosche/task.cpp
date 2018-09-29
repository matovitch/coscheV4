#include "cosche/task.hpp"

#include "cosche/context_switch.hpp"
#include "cosche/scheduler.hpp"

namespace cosche
{

namespace task
{

Abstract::Abstract(Scheduler& scheduler) :
    _scheduler {scheduler}
{}

Abstract::~Abstract() {}

} // namespace task

} // namepsace cosche