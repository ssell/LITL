#include <chrono>
#include <random>

#include "litl-core/math/random/randomMT19937.hpp"

namespace litl
{
    RandomMT19937& RandomMT19937::shared() noexcept
    {
        static thread_local RandomMT19937 rng;
        return rng;
    }

    struct RandomMT19937::Impl
    {
        std::mt19937 rng;
        uint32_t seed;
    };

    RandomMT19937::RandomMT19937()
    {
        seed(0);
    }

    RandomMT19937::RandomMT19937(uint_fast32_t seed)
    {
        this->seed(seed);
    }

    RandomMT19937::~RandomMT19937()
    {

    }

    uint_fast32_t RandomMT19937::operator()() noexcept
    {
        return next();
    }

    uint_fast32_t RandomMT19937::next() noexcept
    {
        return m_impl->rng();
    }

    uint_fast32_t RandomMT19937::next(uint_fast32_t max) noexcept
    {
        return next() % max;
    }

    float RandomMT19937::next01() noexcept
    {
        // No point in using the full range of unsigned value as floats can accurately portray them ...
        return static_cast<float>(next(1000000u)) * 0.000001f;
    }

    void RandomMT19937::discard(uint32_t steps) noexcept
    {
        m_impl->rng.discard(steps);
    }

    void RandomMT19937::seed() noexcept
    {
        seed(static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count()));
    }

    void RandomMT19937::seed(uint_fast32_t seed) noexcept
    {
        m_impl->seed = seed;
        m_impl->rng.seed(seed);
    }

    uint32_t RandomMT19937::getSeed() const noexcept
    {
        return m_impl->seed;
    }

    uint_fast32_t RandomMT19937::min() const noexcept
    {
        return m_impl->rng.min();
    }

    uint_fast32_t RandomMT19937::max() const noexcept
    {
        return m_impl->rng.max();
    }
}