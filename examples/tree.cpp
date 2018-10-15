#include <iostream>
#include <cstdlib>
#include <chrono>

#include "cosche/scheduler.hpp"
#include "cosche/utils.hpp"

static constexpr unsigned TREE_DEPTH = 5;

std::function<void()> makeRecursiveWork(const unsigned treeDepth, cosche::Scheduler& scheduler)
{
    if (treeDepth == 0)
    {
        return
            []()
            {
                std::cout << "Here is a leaf! - treeDepth = 0" << std::endl;
            };
    }

    return
        [treeDepth, &scheduler]()
        {
            auto&& left  = scheduler.makeTask(makeRecursiveWork(treeDepth - 1, scheduler));
            auto&& right = scheduler.makeTask(makeRecursiveWork(treeDepth - 1, scheduler));

            scheduler.attachBatch({&left, &right});

            std::cout << "Here is a node! - treeDepth = " << treeDepth << std::endl;
        };
}

int main()
{
    cosche::Scheduler scheduler;

    scheduler.makeTask(makeRecursiveWork(TREE_DEPTH, scheduler));

    scheduler.run();

    cosche::cleanUp();

    return EXIT_SUCCESS;
}
