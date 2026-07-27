#include <chrono>
#include <random>

#include "litl-core/math/random/randomLCG.hpp"

namespace litl
{
    RandomLCG& RandomLCG::shared() noexcept
    {
        static thread_local RandomLCG rng;
        return rng;
    }

    struct RandomLCG::Impl
    {
        std::minstd_rand rng;
        uint32_t seed;
    };

    RandomLCG::RandomLCG()
    {
        seed(0);
    }

    RandomLCG::RandomLCG(uint_fast32_t seed)
    {
        this->seed(seed);
    }

    RandomLCG::~RandomLCG()
    {

    }

    uint_fast32_t RandomLCG::operator()() noexcept
    {
        return next();
    }

    uint_fast32_t RandomLCG::next() noexcept
    {
        return m_impl->rng();
    }

    uint_fast32_t RandomLCG::next(uint_fast32_t max) noexcept
    {
        return next() % max;
    }

    float RandomLCG::next01() noexcept
    {
        // No point in using the full range of unsigned value as floats can accurately portray them ...
        return static_cast<float>(next(1000000u)) * 0.000001f;
    }

    void RandomLCG::discard(uint32_t steps) noexcept
    {
        m_impl->rng.discard(steps);
    }

    void RandomLCG::seed() noexcept
    {
        seed(static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count()));
    }

    void RandomLCG::seed(uint_fast32_t seed) noexcept
    {
        m_impl->seed = seed;
        m_impl->rng.seed(seed);
    }

    uint32_t RandomLCG::getSeed() const noexcept
    {
        return m_impl->seed;
    }

    uint_fast32_t RandomLCG::min() const noexcept
    {
        return m_impl->rng.min();
    }

    uint_fast32_t RandomLCG::max() const noexcept
    {
        return m_impl->rng.max();
    }
}