#include <iostream>
#include <cstdlib>
#include <chrono>

#include "cosche/scheduler.hpp"
#include "cosche/utils.hpp"

int main()
{
    cosche::Scheduler scheduler;

    scheduler.makeTask<void>
        (
            [&]()
            {
                auto&& nestedTask = scheduler.makeTask<void>
                    (
                        []()
                        {
                            std::cout << "Inside!" << std::endl;
                        }
                    );

                scheduler.attach(nestedTask);

                std::cout << "Outside!" << std::endl;
            }
        );

    scheduler.run();

    cosche::cleanUp();

    return EXIT_SUCCESS;
}
