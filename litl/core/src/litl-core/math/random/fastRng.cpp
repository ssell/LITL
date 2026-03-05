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

    void FastRng::discard(uint32_t steps) noexcept
    {
        m_impl->rng.discard(steps);
    }

    void FastRng::seed() noexcept
    {
        m_impl->rng.seed(std::chrono::system_clock::now().time_since_epoch().count());
    }

    void FastRng::seed(uint_fast32_t seed) noexcept
    {
        m_impl->rng.seed(seed);
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