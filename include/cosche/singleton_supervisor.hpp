#pragma once

#include <vector>

namespace cosche
{

template <class TYPE>
class TSingleton;

namespace singleton
{

struct Abstract;

class Supervisor
{

public:

    template <class TYPE>
    static void registerSingleton()
    {
        _singletons.push_back(new TSingleton<TYPE>);
    }

    static void clean();

private:

    static std::vector<Abstract*> _singletons;
};

} // namespace singleton

} // namespace cosche
