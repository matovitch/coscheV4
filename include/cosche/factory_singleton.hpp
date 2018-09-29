#pragma once

#include "cosche/singleton.hpp"
#include "cosche/factory.hpp"

namespace cosche
{

template <class Type>
using TFactorySingleton = TSingleton<TFactory<Type>>;

} // namespace cosche