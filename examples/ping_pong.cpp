#include <functional>
#include <iostream>
#include <cstdlib>
#include <future>
#include <chrono>
#include <thread>

#include "cosche/scheduler.hpp"
#include "cosche/utils.hpp"

int main()
{
    cosche::Scheduler scheduler;

    auto&& pingTask = scheduler.makeTask<void>();

    std::function<void()> pingWork =
        [&]()
        {
            auto&& pongTask = scheduler.makeTask<void>
                (
                    [&]()
                    {
                        std::cout << "Pong!" << std::endl;
                        scheduler.detach(pingTask);

                        using namespace std::chrono_literals;

                        auto&& answer = scheduler.attach
                            (
                                std::async
                                (
                                    []
                                    {
                                        std::this_thread::sleep_for(2s);
                                        return 42;
                                    }
                                ),
                                10us, // polling delay to avoid maxing out CPU
                                2s    // timeout
                            );

                        std::cout << (answer ? *answer : -1) << std::endl;

                        std::cout << "Pong!" << std::endl;
                    }
                );

            std::cout << "Ping!" << std::endl;

            scheduler.attach(pingTask, pongTask);

            std::cout << "Ping! Waiting for the other thread"
                        " to return 42 or -1 on timeout..." << std::endl;

            scheduler.detach(pongTask);
            scheduler.attach(pongTask);
        };

    cosche::assignWork(pingTask, std::move(pingWork));

    scheduler.run();

    cosche::cleanUp();

    return EXIT_SUCCESS;
}
