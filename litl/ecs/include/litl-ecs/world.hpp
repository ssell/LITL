#ifndef LITL_ENGINE_ECS_WORLD_H__
#define LITL_ENGINE_ECS_WORLD_H__

#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>

#include "litl-ecs/entity.hpp"
#include "litl-ecs/entityRecord.hpp"
#include "litl-ecs/component.hpp"
#include "litl-ecs/archetype/archetype.hpp"
#include "litl-ecs/system/systemScheduler.hpp"

namespace LITL::ECS
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
        /// Returns the record associated with the specified entity.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        EntityRecord getEntityRecord(Entity entity) const noexcept;

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
        template<ValidComponentType ComponentType>
        void addComponentImmediate(Entity entity) const noexcept
        {
            addComponentImmediate(entity, ComponentDescriptor::get<ComponentType>()->id);
        }

        /// <summary>
        /// Immediately adds the specified component to the Entity.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <returns></returns>
        template<ValidComponentType ComponentType>
        void addComponentImmediate(Entity entity, ComponentType const& component)
        {
            addComponentImmediate<ComponentType>(entity);
            setComponent(entity, component);
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
        template<ValidComponentType... ComponentTypes>
        void addComponentsImmediate(Entity entity) const noexcept
        {
            std::vector<ComponentTypeId> componentTypeIds;
            componentTypeIds.reserve(sizeof...(ComponentTypes));
            (foldComponentTypesIntoVector<ComponentTypes>(componentTypeIds), ...);

            addComponentsImmediate(entity, componentTypeIds);
        }

        /// <summary>
        /// Immediately adds the specified components to the Entity with the provided values.
        /// 
        /// It is recommended to use an ECS Command Buffer instead. The use of this, 
        /// or other *Immediate methods, should be limited to setting up simple demos, tests, etc.
        /// </summary>
        /// <typeparam name="...ComponentTypes"></typeparam>
        /// <param name="entity"></param>
        /// <param name="...Args"></param>
        template<ValidComponentType... ComponentTypes>
        void addComponentsImmediate(Entity entity, ComponentTypes&& ... Args)
        {
            (addComponentImmediate<ComponentTypes>(entity), ...);
            (setComponent(entity, Args), ...);
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
        template<ValidComponentType ComponentType>
        void removeComponentImmediate(Entity entity) const noexcept
        {
            removeComponentImmediate(entity, ComponentDescriptor::get<ComponentType>()->id);
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
        template<ValidComponentType... ComponentTypes>
        void removeComponentsImmediate(Entity entity) const noexcept
        {
            std::vector<ComponentTypeId> componentTypeIds;
            componentTypeIds.reserve(sizeof...(ComponentTypes));
            (foldComponentTypesIntoVector<ComponentTypes>(componentTypeIds), ...);

            removeComponentsImmediate(entity, componentTypeIds);
        }

        /// <summary>
        /// Returns the specified component on the Entity if it exists.
        /// 
        /// Note: that it is generally unusual to retrieve a component in this manner as 
        /// components should typically be processed iteratively in a system.
        /// 
        /// Note Note: Modifying the returned component does NOT modify the component
        /// stored within the archetype chunk (aka it does nothing). To update the 
        /// component use the setComponent method. This is intentional as a retrieval
        /// via this method may be done at an unsafe time and so any modifications
        /// should also be intentionally (and thoughtfully) done.
        /// </summary>
        /// <typeparam name="ComponentType"></typeparam>
        /// <param name="entity"></param>
        /// <returns></returns>
        template<ValidComponentType ComponentType>
        std::optional<ComponentType> getComponent(Entity entity) const noexcept
        {
            if (!isAlive(entity))
            {
                return std::nullopt;
            }

            const auto record = getEntityRecord(entity);
            
            if (!record.archetype->hasComponent<ComponentType>())
            {
                return std::nullopt;
            }

            return record.archetype->getComponent<ComponentType>(record);
        }

        /// <summary>
        /// Sets the value of the specified component for the Entity if it exists.
        /// 
        /// Note: that it is generally unusual to set a component value in this manner as
        /// components should typically be processed iteratively in a system.
        /// </summary>
        /// <typeparam name="ComponentType"></typeparam>
        /// <param name="entity"></param>
        /// <param name="component"></param>
        template<ValidComponentType ComponentType>
        void setComponent(Entity entity, ComponentType const& component) noexcept
        {
            if (!isAlive(entity))
            {
                return;
            }

            const auto record = getEntityRecord(entity);

            if (!record.archetype->hasComponent<ComponentType>())
            {
                return;
            }

            record.archetype->setComponent<ComponentType>(record, component);
        }

        template<ValidSystem S>
        void addSystem(SystemGroup group) noexcept
        {
            getSystemScheduler().addSystem<S>(group);
        }

    protected:

    private:

        SystemScheduler& getSystemScheduler() noexcept;

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif