#pragma once

#include "cosche/factory_singleton.hpp"
#include "cosche/future_task_pair.hpp"
#include "cosche/context_switch.hpp"
#include "cosche/coroutine.hpp"
#include "cosche/register.hpp"
#include "cosche/future.hpp"
#include "cosche/graph.hpp"
#include "cosche/task.hpp"

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

    Scheduler() :
        _futuresPollDelay{10000 /*in nanoseconds*/}
    {
        mmxFpuSave(&MMX_FPU_STATE);
    }

    template <class Rep, class Period>
    Scheduler(const std::chrono::duration<Rep, Period>& futuresPollDelay) :
        _futuresPollDelay{futuresPollDelay}
    {
        mmxFpuSave(&MMX_FPU_STATE);
    }

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

    template <class ReturnType>
    ReturnType attach(TaskNode& taskNode,
                      std::future<ReturnType>&& future)
    {
        using Future = TFuture<ReturnType>;

        auto&& theFuture = TFactorySingleton<Future>::instance().make(std::move(future));
        _futuresTaskPairs.emplace_back(&theFuture, &taskNode);

        attach(taskNode,
               taskNode);

        return theFuture.value().get();
    }

    template <class ReturnType>
    ReturnType attach(std::future<ReturnType>&& future)
    {
        if (COSCHE_UNLIKELY(_me == nullptr))
        {
            return future.get();
        }

        return attach(*_me, std::move(future));
    }

    template <class ReturnType, class Rep, class Period>
    std::optional<ReturnType> attach(TaskNode& taskNode,
                                     std::future<ReturnType>&& future,
                                     const std::chrono::duration<Rep, Period>& timeoutDuration)
    {
        using Future = future::TWithTimeout<ReturnType>;

        auto&& theFuture = TFactorySingleton<Future>::instance().make(std::move(future),
                                                                      timeoutDuration);
        _futuresTaskPairs.emplace_back(&theFuture, &taskNode);

        attach(taskNode,
               taskNode);

        using namespace std::chrono_literals;

        return (theFuture.value().wait_for(0s) == std::future_status::ready) ?
            std::optional<ReturnType>{theFuture.value().get()}               :
            std::optional<ReturnType>{};
    }

    template <class ReturnType, class Rep, class Period>
    std::optional<ReturnType> attach(std::future<ReturnType>&& future,
                                     const std::chrono::duration<Rep, Period>& timeoutDuration)
    {
        if (COSCHE_UNLIKELY(_me == nullptr))
        {
            return {};
        }

        return attach(*_me, std::move(future), timeoutDuration);
    }

private:

    static void taskEntryPoint();

    bool hasFutures();

    void releaseContext(TaskNode& taskNode);

    TaskGraph                   _taskGraph;
    std::vector<FutureTaskPair> _futuresTaskPairs;

    std::chrono::nanoseconds _futuresPollDelay;

    static thread_local Coroutine _coroutine;
    static thread_local TaskNode* _me;
};

} // namespace cosche
