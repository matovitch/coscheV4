#include "cosche/future_task_pair.hpp"

namespace cosche
{

FutureTaskPair::FutureTaskPair(future::Abstract       * const futurePtr,
                               TNode<task::Abstract*> * const taskNodePtr) :
    _futurePtr   { futurePtr   },
    _taskNodePtr { taskNodePtr }
{}

} // namespace cosche