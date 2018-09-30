#pragma once

#include <future>
#include <chrono>

namespace cosche
{

namespace future
{

struct Abstract
{
    virtual bool ready() const = 0;

    virtual ~Abstract();
};

template <class Type>
class TWithTimeout : public Abstract
{

public:

    template <class Rep2, class Period2>
    TWithTimeout(std::future<Type>&&                         future,
                 const std::chrono::duration<Rep2, Period2>& timeout) :
        _future       { std::move(future)                          },
        _timeout      { std::chrono::steady_clock::now() + timeout }
    {}

    bool ready() const override
    {
        using namespace std::chrono_literals;

        return _future.wait_for(0s) == std::future_status::ready ||
               std::chrono::steady_clock::now() > _timeout;
    }

    std::future<Type>& value()
    {
        return _future;
    }

private:

    std::future<Type>                           _future;
    const std::chrono::steady_clock::time_point _timeout;
};

} // namespace future

template <class Type>
class TFuture : public future::Abstract
{

public:

    TFuture(std::future<Type>&& future) :
        _future{std::move(future)}
    {}

    bool ready() const override
    {
        using namespace std::chrono_literals;

        return _future.wait_for(0s) == std::future_status::ready;
    }

    std::future<Type>& value()
    {
        return _future;
    }

private:

    std::future<Type> _future;
};

} // namespace cosche
