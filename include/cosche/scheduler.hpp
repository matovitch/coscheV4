#pragma once

#include "factory_singleton.hpp"
#include "future_task_pair.hpp"
#include "coroutine.hpp"
#include "future.hpp"
#include "graph.hpp"
#include "task.hpp"

#include <optional>
#include <vector>
#include <chrono>

#include <iostream>

namespace cosche
{

class Scheduler
{
    friend struct Coroutine;

    using TaskGraph = TGraph<task::Abstract*>;

public:

    using TaskNode  = typename TaskGraph::Node;

    Scheduler();

    void attach(TaskNode& lhs,
                TaskNode& rhs);

    void detach(TaskNode& lhs,
                TaskNode& rhs);

    void attach(TaskNode& dependee);

    void detach(TaskNode& depender);

    void detachAll(TaskNode& depender);

    void detachAll();

    void run();

    template<class ReturnType>
    TaskNode& makeTask()
    {
        using Task = TTask<ReturnType>;

        Task& task = TFactorySingleton<Task>::instance().make(*this);

        return _taskGraph.makeNode(&task);
    }

    template<class ReturnType>
    TaskNode& makeTask(std::function<ReturnType()>&& taskWork)
    {
        using Task = TTask<ReturnType>;

        Task& task = TFactorySingleton<Task>::instance().make(*this);

        task.assign(std::move(taskWork));

        return _taskGraph.makeNode(&task);
    }

    void attachBatch(TaskNode& taskNode, const std::vector<TaskNode*>& dependees);
    void attachBatch(                    const std::vector<TaskNode*>& dependees);

    /* Due to http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#1591,
       template deduction doesn't work with std::array. */
    template <std::size_t BATCH_SIZE>
    void attachBatch(TaskNode& taskNode, TaskNode* const (&dependees)[BATCH_SIZE])
    {
        _taskGraph.attachBatch(taskNode, dependees);

        releaseContext(taskNode);
    }

    template <std::size_t BATCH_SIZE>
    void attachBatch(TaskNode* const (&dependees)[BATCH_SIZE])
    {
        if (COSCHE_UNLIKELY(_me == nullptr))
        {
            return;
        }

        attachBatch(*_me, dependees);
    }

    template <class ReturnType, class Rep, class Period>
    ReturnType attach(TaskNode& taskNode,
                      std::future<ReturnType>&& future,
                      const std::chrono::duration<Rep, Period>& pollingDelay)
    {
        using Future = TFuture<ReturnType, Rep, Period>;

        auto&& theFuture = TFactorySingleton<Future>::instance().make(std::move(future),
                                                                      pollingDelay);
        _futuresTaskPairs.emplace_back(&theFuture, &taskNode);

        attach(taskNode,
               taskNode);

        return theFuture.value().get();
    }

    template <class ReturnType, class Rep, class Period>
    ReturnType attach(std::future<ReturnType>&& future,
                      const std::chrono::duration<Rep, Period>& pollingDelay)
    {
        if (COSCHE_UNLIKELY(_me == nullptr))
        {
            return future.get();
        }

        return attach(*_me, std::move(future), pollingDelay);
    }

    template <class ReturnType, class Rep1, class Period1,
                                class Rep2, class Period2>
    std::optional<ReturnType> attach(TaskNode& taskNode,
                                     std::future<ReturnType>&& future,
                                     const std::chrono::duration<Rep1, Period1>& pollingDelay,
                                     const std::chrono::duration<Rep2, Period2>& timeoutDuration)
    {
        using Future = future::TScoped<ReturnType, Rep1, Period1>;

        auto&& theFuture = TFactorySingleton<Future>::instance().make(std::move(future),
                                                                      pollingDelay,
                                                                      timeoutDuration);
        _futuresTaskPairs.emplace_back(&theFuture, &taskNode);

        attach(taskNode,
               taskNode);

        using namespace std::chrono_literals;

        return (theFuture.value().wait_for(0s) == std::future_status::ready) ?
            std::optional<ReturnType>{theFuture.value().get()}               :
            std::optional<ReturnType>{};
    }

    template <class ReturnType, class Rep1, class Period1,
                                class Rep2, class Period2>
    std::optional<ReturnType> attach(std::future<ReturnType>&& future,
                                     const std::chrono::duration<Rep1, Period1>& pollingDelay,
                                     const std::chrono::duration<Rep2, Period2>& timeoutDuration)
    {
        if (COSCHE_UNLIKELY(_me == nullptr))
        {
            return {};
        }

        return attach(*_me, std::move(future), pollingDelay, timeoutDuration);
    }

private:

    static void taskEntryPoint();

    bool hasFutures();

    void releaseContext(TaskNode& taskNode);

    TaskGraph                   _taskGraph;
    std::vector<FutureTaskPair> _futuresTaskPairs;

    static thread_local Coroutine _coroutine;
    static thread_local TaskNode* _me;
};

} // namespace cosche