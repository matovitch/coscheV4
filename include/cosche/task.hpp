#pragma once

#include "cosche/factory_singleton.hpp"
#include "cosche/coroutine.hpp"
#include "cosche/stack.hpp"

#include <functional>
#include <utility>
#include <cstdint>
#include <future>

namespace cosche
{

class Scheduler;

namespace task
{

class Abstract
{

    friend class cosche::Scheduler;

public:

    static void entryPoint();

    Abstract(Scheduler& scheduler);

    virtual void run() = 0;

    virtual ~Abstract();

    Scheduler& _scheduler;
    Coroutine  _coroutine;

    static thread_local Abstract* TO_RUN;
};

} // namespace task

template<class ReturnType>
class TTask : public task::Abstract
{

public:

    TTask(Scheduler& scheduler) : task::Abstract{scheduler} {}

    void assign(std::function<ReturnType()>&& function)
    {
        _functionOpt.emplace(std::move(function));
    }

    void run()
    {
        if (!_functionOpt)
        {
            return;
        }

        _promise.set_value((_functionOpt.value())());

        TFactorySingleton<TTask<ReturnType>>::instance().recycle(this);
    }

    std::future<ReturnType> future()
    {
        return _promise.getFuture();
    }

private:

    std::optional<std::function<ReturnType()>> _functionOpt;
    std::promise<ReturnType>                   _promise;
};


template<>
class TTask<void> : public task::Abstract
{

public:

    TTask(Scheduler& scheduler) : task::Abstract{scheduler} {}

    void assign(std::function<void()>&& function)
    {
        _functionOpt.emplace(std::move(function));
    }

    void run()
    {
        if (!_functionOpt)
        {
            return;
        }

        (*_functionOpt)();

        TFactorySingleton<TTask<void>>::instance().recycle(this);
    }

private:

    std::optional<std::function<void()>> _functionOpt;
};

} // namespace cosche