#ifndef LITL_ENGINE_ECS_WORLD_H__
#define LITL_ENGINE_ECS_WORLD_H__

#include <cstdint>
#include <type_traits>

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
        Entity createImmediate() const noexcept;

        /// <summary>
        /// Immediately destroys the Entity.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <param name="entity"></param>
        void destroyImmediate(Entity entity) const noexcept;

        /// <summary>
        /// Checks if the entity is alive and can be operated on.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        bool isAlive(Entity entity) const noexcept;

        /// <summary>
        /// Returns the number of components attached to this entity.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        uint32_t componentCount(Entity entity) const noexcept;

        /// <summary>
        /// Immediately adds the specified component to the Entity.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="component"></param>
        void addComponentImmediate(Entity entity, ComponentTypeId component) const noexcept;

        /// <summary>
        /// Immediately adds the specified component to the Entity.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <returns></returns>
        template<typename T>
        void addComponentImmediate(Entity entity) const noexcept
        {
            static_assert(std::is_standard_layout_v<T>);
            addComponentImmediate(entity, ComponentDescriptor::get<T>()->id);
        }

        /// <summary>
        /// Immediately adds the specified components to the Entity.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="components"></param>
        void addComponentsImmediate(Entity entity, std::vector<ComponentTypeId>& components) const noexcept;

        /// <summary>
        /// Immediately adds the specified components to the Entity.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <typeparam name="...ComponentTypes"></typeparam>
        /// <param name="entity"></param>
        template<typename... ComponentTypes>
        void addComponentsImmediate(Entity entity) const noexcept
        {
            static_assert((std::is_standard_layout_v<ComponentTypes> && ...));

            std::vector<ComponentTypeId> componentTypeIds;
            componentTypeIds.reserve(sizeof...(ComponentTypes));
            (foldComponentTypesIntoVector<ComponentTypes>(componentTypeIds), ...);

            addComponentsImmediate(entity, componentTypeIds);
        }

        /// <summary>
        /// Immediately removes the specified component from the Entity.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="component"></param>
        void removeComponentImmediate(Entity entity, ComponentTypeId component) const noexcept;

        /// <summary>
        /// Immediately removes the specified component from the Entity.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="entity"></param>
        template<typename T>
        void removeComponentImmediate(Entity entity) const noexcept
        {
            static_assert(std::is_standard_layout_v<T>);
            removeComponentImmediate(entity, ComponentDescriptor::get<T>()->id);
        }

        /// <summary>
        /// Immediately removed the specified components from the Entity.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="components"></param>
        void removeComponentsImmediate(Entity entity, std::vector<ComponentTypeId>& components) const noexcept;

        /// <summary>
        /// Immediately removed the specified components from the Entity.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <typeparam name="...ComponentTypes"></typeparam>
        /// <param name="entity"></param>
        template<typename... ComponentTypes>
        void removeComponentsImmediate(Entity entity) const noexcept
        {
            static_assert((std::is_standard_layout_v<ComponentTypes> && ...));

            std::vector<ComponentTypeId> componentTypeIds;
            componentTypeIds.reserve(sizeof...(ComponentTypes));
            (foldComponentTypesIntoVector<ComponentTypes>(componentTypeIds), ...);

            removeComponentsImmediate(entity, componentTypeIds);
        }

    protected:

    private:

    };
}

#endif