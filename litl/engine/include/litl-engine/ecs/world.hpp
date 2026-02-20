#ifndef LITL_ENGINE_ECS_WORLD_H__
#define LITL_ENGINE_ECS_WORLD_H__

#include "litl-engine/ecs/entity.hpp"
#include "litl-engine/ecs/component.hpp"

namespace LITL::Engine::ECS
{
    /// <summary>
    /// Single point of entry for general ECS operations.
    /// Users should make use of World instead of the individual registries and other constructs if possible.
    /// </summary>
    class World final
    {
    public:

        World();
        World(World const&) = delete;
        World& operator=(World const&) = delete;
        ~World();

        /// <summary>
        /// Immediately creates a new Entity.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <returns></returns>
        Entity createImmediate() noexcept;

        /// <summary>
        /// Immediately adds the specified component to the Entity.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="component"></param>
        void addComponentImmediate(Entity entity, ComponentTypeId component);

        /// <summary>
        /// Immediately adds the specified component to the Entity.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <returns></returns>
        template<typename T>
        void addComponentImmediate(Entity entity)
        {
            addComponentImmediate(ComponentDescriptor::get<T>()->id);
        }

        /// <summary>
        /// Immediately adds the specified components to the Entity.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="components"></param>
        void addComponentsImmediate(Entity entity, std::vector<ComponentTypeId>& components);

        /// <summary>
        /// Immediately adds the specified components to the Entity.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <typeparam name="...ComponentTypes"></typeparam>
        /// <param name="entity"></param>
        template<typename... ComponentTypes>
        void addComponentsImmediate(Entity entity)
        {
            std::vector<ComponentTypeId> componentTypeIds;
            componentTypeIds.reserve(sizeof...(ComponentTypes));
            (foldComponentTypesIntoVector<ComponentTypes>(componentTypeIds), ...);

            addComponentsImmediate(entity, componentTypeIds);
        }

    protected:

    private:

    };
}

#endif