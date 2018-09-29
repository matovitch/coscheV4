#include "cosche/singleton_supervisor.hpp"
#include "cosche/singleton.hpp"

namespace cosche
{

namespace singleton
{

void Supervisor::clean()
{
    for (auto singleton : _singletons)
    {
        singleton->clean();
        delete singleton;
    }
}

std::vector<Abstract*> Supervisor::_singletons;

} // namespace singleton

} // namespace cosche
