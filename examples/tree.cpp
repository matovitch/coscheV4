#include <iostream>
#include <cstdlib>

#include "cosche/cosche.hpp"

static constexpr unsigned TREE_DEPTH = 3;

using TaskNode = typename cosche::Scheduler::TaskNode;

std::function<void()> makeRecursiveWork(const unsigned treeDepth, cosche::Scheduler& scheduler, TaskNode& task)
{
    if (treeDepth == 0)
    {
        return
            []()
            {
                std::cout << "Here is a leaf!" << std::endl;
            };
    }

    return
        [treeDepth, &scheduler, &task]()
        {
            auto&& left  = scheduler.makeTask<void>();
            auto&& right = scheduler.makeTask<void>();

            cosche::assignWork(left  , makeRecursiveWork(treeDepth - 1, scheduler, left  ));
            cosche::assignWork(right , makeRecursiveWork(treeDepth - 1, scheduler, right ));

            scheduler.attachBatch(task, {&left, &right});

            std::cout << "Here is a node!" << std::endl;
        };
}

int main()
{
    cosche::Scheduler scheduler;

    auto&& rootTask = scheduler.makeTask<void>();

    auto&& rootWork = makeRecursiveWork(TREE_DEPTH, scheduler, rootTask);

    cosche::assignWork(rootTask, std::move(rootWork));

    scheduler.run();

    cosche::cleanUp();

    return EXIT_SUCCESS;
}
