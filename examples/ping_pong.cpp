#include <iostream>
#include <cstdlib>

#include "cosche/cosche.hpp"

int main()
{
    cosche::Scheduler scheduler;

    auto&& pingTask = scheduler.makeTask<void>();
    auto&& pongTask = scheduler.makeTask<void>();

    std::function<void()> pingWork =
        [&]()
        {
            std::cout << "ping" << std::endl;
            scheduler.detach(pongTask, pingTask);
            scheduler.attach(pingTask, pongTask);
            std::cout << "ping" << std::endl;
            scheduler.detach(pongTask, pingTask);
            scheduler.attach(pingTask, pongTask);

            using namespace std::chrono_literals;

            auto&& answer = scheduler.attach(pingTask,
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

            std::cout << "ping" << std::endl;
            scheduler.detach(pongTask, pingTask);
            scheduler.attach(pingTask, pongTask);
            std::cout << "ping" << std::endl;
        };

    std::function<void()> pongWork =
        [&]()
        {
            std::cout << "pong" << std::endl;
            scheduler.detach(pingTask, pongTask);
            scheduler.attach(pongTask, pingTask);
            std::cout << "pong" << std::endl;
            scheduler.detach(pingTask, pongTask);
            scheduler.attach(pongTask, pingTask);
            std::cout << "pong" << std::endl;
        };

    cosche::assignWork(pingTask, std::move(pingWork));
    cosche::assignWork(pongTask, std::move(pongWork));

    scheduler.attach(pongTask, pingTask); // to be sure to start with ping

    scheduler.run();

    cosche::cleanUp();

    return EXIT_SUCCESS;
}
