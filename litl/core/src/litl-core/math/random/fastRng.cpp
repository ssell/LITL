#include <chrono>
#include <random>

#include "litl-core/math/random/fastRng.hpp"

namespace LITL::Math
{
    FastRng& FastRng::shared() noexcept
    {
        static thread_local FastRng rng;
        return rng;
    }

    struct FastRng::Impl
    {
        std::minstd_rand rng;
        uint32_t seed;
    };

    FastRng::FastRng()
    {
        seed(0);
    }

    FastRng::FastRng(uint_fast32_t seed)
    {
        this->seed(seed);
    }

    FastRng::~FastRng()
    {

    }

    uint_fast32_t FastRng::operator()() noexcept
    {
        return next();
    }

    uint_fast32_t FastRng::next() noexcept
    {
        return m_impl->rng();
    }

    uint_fast32_t FastRng::next(uint_fast32_t max) noexcept
    {
        return next() % max;
    }

    void FastRng::discard(uint32_t steps) noexcept
    {
        m_impl->rng.discard(steps);
    }

    void FastRng::seed() noexcept
    {
        seed(static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count()));
    }

    void FastRng::seed(uint_fast32_t seed) noexcept
    {
        m_impl->seed = seed;
        m_impl->rng.seed(seed);
    }

    uint32_t FastRng::getSeed() const noexcept
    {
        return m_impl->seed;
    }

    uint_fast32_t FastRng::min() const noexcept
    {
        return m_impl->rng.min();
    }

    uint_fast32_t FastRng::max() const noexcept
    {
        return m_impl->rng.max();
    }
}