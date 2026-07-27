#ifndef LITL_MATH_RANDOM_H__
#define LITL_MATH_RANDOM_H__

#include "litl-core/math/random/randomLCG.hpp"
#include "litl-core/math/random/randomMT19937.hpp"

namespace litl
{
    using Random = RandomMT19937;
    using RandomFast = RandomLCG;
}

#endif