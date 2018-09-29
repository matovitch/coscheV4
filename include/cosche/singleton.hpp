#pragma once

#include "singleton_supervisor.hpp"
#include "likelyhood.hpp"

namespace cosche
{

namespace singleton
{

struct Abstract
{
    virtual void clean() = 0;

    virtual ~Abstract();
};

} //namespace singleton

template <class TYPE>
class TSingleton : public singleton::Abstract
{

public:

    static TYPE& instance()
    {
        using namespace singleton;

        if (COSCHE_UNLIKELY(_instance == nullptr))
        {
            Supervisor::registerSingleton<TYPE>();
            _instance = new TYPE;
        }

        return *_instance;
    }

    void clean() override
    {
        delete _instance;
        _instance = nullptr;
    }

private:

    static TYPE* _instance;
};

template <class TYPE>
TYPE* TSingleton<TYPE>::_instance = nullptr;

} // namespace cosche
