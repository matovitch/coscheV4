#pragma once

#include "singleton.hpp"
#include "factory.hpp"

namespace cosche
{

template <class Type>
using TFactorySingleton = TSingleton<TFactory<Type>>;

} // namespace cosche