#include "cosche/cosche.hpp"

#include "cosche/singleton_supervisor.hpp"

namespace cosche
{

void cleanUp()
{
    singleton::Supervisor::clean();
}

} // namespace cosche
