#ifndef LITL_ENGINE_ECS_SYSTEM_RUNNER_H__
#define LITL_ENGINE_ECS_SYSTEM_RUNNER_H__

#include <tuple>

#include "litl-ecs/system/systemTraits.hpp"

namespace LITL::ECS
{
    class World;
    struct ChunkLayout;

    /// <summary>
    /// The function signature required to run the SystemRunner.
    /// </summary>
    using SystemRunFunc = void(LITL::ECS::World&, float, LITL::ECS::Chunk&, LITL::ECS::ChunkLayout const&);

    /// <summary>
    /// Responsible for running a system over a single archetype chunk.
    /// The chunk is expected to have all of the components required by the system.
    /// </summary>
    /// <typeparam name="System"></typeparam>
    template<ValidSystem S>
    class SystemRunner
    {
    public:

        explicit SystemRunner(S* system)
            : m_pSystem(system)
        {

        }

        // Must match SystemRunFunc
        void run(World& world, float dt, Chunk& chunk, ChunkLayout const& layout)
        {
            // Get the system components in tuple form. For example: std::tuple<Foo&, Bar&>
            using SystemComponentTuple = SystemComponents<S>;
            iterate<SystemComponentTuple>(world, dt, chunk, layout);
        }

    protected:

    private:

        template<typename SystemComponentTuple>
        void iterate(World& world, float dt, Chunk& chunk, ChunkLayout const& layout)
        {
            // Retrieve the data ptr for each component in the tuple type.
            // For example: SystemComponentTuple -> std::tuple<Foo&, Bar&> ->
            //    componentArrays[0] = Foo*
            //    componentArrays[1] = Bar*
            // Ends with: std::tuple<Foo*, Bar*>
            auto componentArrays = SystemComponentsTupleOperations<SystemComponentTuple>::extractComponentBuffers(chunk, layout);

            const uint32_t entityCount = chunk.size();

            // Call System::update for each entity in the chunk.
            for (uint32_t i = 0; i < entityCount; ++i)
            {
                // Use apply to expand the tuple into parameters.
                // Applies the provded lambda to each member of the tuple.
                std::apply([&](auto&... componentArray)
                    {
                        m_pSystem->update(world, dt, componentArray[i]...);
                    }, componentArrays);
            }
        }

        S* m_pSystem;
    };
}

#endif