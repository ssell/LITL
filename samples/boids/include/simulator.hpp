#ifndef LITL_SAMPLES_BOIDS_SIMULATOR_H__
#define LITL_SAMPLES_BOIDS_SIMULATOR_H__

namespace litl
{
    /// <summary>
    /// The simulator for the boids sample.
    /// Responsible for spawning food and additional boids if the population gets low.
    /// </summary>
    class Simulator
    {
    public:

        void update(float dt) noexcept;

    private:
    };
}

#endif