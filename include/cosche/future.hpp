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

template <class T, class Rep1, class Period1>
class TScoped : public Abstract
{

public:

    template <class Rep2, class Period2>
    TScoped(std::future<T>&&                            future,
            const std::chrono::duration<Rep2, Period2>& timeout,
            const std::chrono::duration<Rep1, Period1>& pollingDelay) :
        _future       { std::move(future)                          },
        _timeout      { std::chrono::steady_clock::now() + timeout },
        _pollingDelay { pollingDelay                               }
    {}

    bool ready() const override
    {
        return _future.wait_for(_pollingDelay) == std::future_status::ready ||
               std::chrono::steady_clock::now() > _timeout;
    }

    std::future<T>& value()
    {
        return _future;
    }

private:

    std::future<T>                              _future;
    const std::chrono::steady_clock::time_point _timeout;
    const std::chrono::duration<Rep1, Period1>  _pollingDelay;
};

} // namespace future

template <class T, class Rep, class Period>
struct TFuture : public future::Abstract
{

public:

    TFuture(std::future<T>&&                          future,
            const std::chrono::duration<Rep, Period>& pollingDelay) :
        _future       { std::move(future) },
        _pollingDelay { pollingDelay      }
    {}

    bool ready() const override
    {
        return _future.wait_for(_pollingDelay) == std::future_status::ready;
    }

    std::future<T>& value()
    {
        return _future;
    }

private:

    std::future<T>                           _future;
    const std::chrono::duration<Rep, Period> _pollingDelay;
};

} // namespace cosche
