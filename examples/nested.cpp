#include <iostream>
#include <cstdlib>

#include "cosche/scheduler.hpp"
#include "cosche/utils.hpp"

int main()
{
    cosche::Scheduler scheduler;

    auto&& rootTask = scheduler.makeTask<void>();

    std::function<void()> rootWork =
        [&]()
        {
            auto&& leafTask = scheduler.makeTask<void>();

            std::function<void()> leafWork =
                []()
                {
                    std::cout << "Inside!" << std::endl;
                };

            cosche::assignWork(leafTask, std::move(leafWork));

            scheduler.attach(rootTask, leafTask);

            std::cout << "Outside!" << std::endl;
        };

    cosche::assignWork(rootTask, std::move(rootWork));

    scheduler.run();

    cosche::cleanUp();

    return EXIT_SUCCESS;
}
