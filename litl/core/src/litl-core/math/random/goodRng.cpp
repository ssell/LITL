#include <chrono>
#include <random>

#include "litl-core/math/random/goodRng.hpp"

namespace litl
{
    GoodRng& GoodRng::shared() noexcept
    {
        static thread_local GoodRng rng;
        return rng;
    }

    struct GoodRng::Impl
    {
        std::mt19937 rng;
        uint32_t seed;
    };

    GoodRng::GoodRng()
    {
        seed(0);
    }

    GoodRng::GoodRng(uint_fast32_t seed)
    {
        this->seed(seed);
    }

    GoodRng::~GoodRng()
    {

    }

    uint_fast32_t GoodRng::operator()() noexcept
    {
        return next();
    }

    uint_fast32_t GoodRng::next() noexcept
    {
        return m_impl->rng();
    }

    uint_fast32_t GoodRng::next(uint_fast32_t max) noexcept
    {
        return next() % max;
    }

    void GoodRng::discard(uint32_t steps) noexcept
    {
        m_impl->rng.discard(steps);
    }

    void GoodRng::seed() noexcept
    {
        seed(static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count()));
    }

    void GoodRng::seed(uint_fast32_t seed) noexcept
    {
        m_impl->seed = seed;
        m_impl->rng.seed(seed);
    }

    uint32_t GoodRng::getSeed() const noexcept
    {
        return m_impl->seed;
    }

    uint_fast32_t GoodRng::min() const noexcept
    {
        return m_impl->rng.min();
    }

    uint_fast32_t GoodRng::max() const noexcept
    {
        return m_impl->rng.max();
    }
}