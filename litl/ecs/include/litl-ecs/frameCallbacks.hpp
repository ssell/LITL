#ifndef LITL_ECS_FRAME_CALLBACKS_H__
#define LITL_ECS_FRAME_CALLBACKS_H__

#include <array>
#include <span>
#include <vector>

#include "litl-core/inlineFunc.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/system/systemGroup.hpp"
#include "litl-ecs/entity/entityCommand.hpp"

namespace litl
{
    /// <summary>
    /// Collection of callbacks that are invoked throughout the lifetime of a single Frame.
    /// </summary>
    struct FrameCallbacks
    {
        static constexpr uint32_t GroupCount = static_cast<uint32_t>(SystemGroup::Count);

        inline_function<void(ServiceProvider&, float)> onFrameStart;
        inline_function<void(ServiceProvider&, float)> onFrameEnd;
        inline_function<void(ServiceProvider&, float)> onRender;
        inline_function<void(ServiceProvider&, SystemGroup, std::span<EntityChange const>)> onSyncPoint;

        std::array<inline_function<void(ServiceProvider&, float, SystemGroup)>, GroupCount> onPreGroup;

        void invokeFrameStart(ServiceProvider& services, float dt) const noexcept
        {
            if (onFrameStart)
            {
                onFrameStart(services, dt);
            }
        }

        void invokeRender(ServiceProvider& services, float dt) const noexcept
        {
            if (onRender)
            {
                onRender(services, dt);
            }
        }

        void invokeFrameEnd(ServiceProvider& services, float dt) const noexcept
        {
            if (onFrameEnd)
            {
                onFrameEnd(services, dt);
            }
        }

        void invokePreGroup(ServiceProvider& services, float dt, SystemGroup group) const noexcept
        {
            auto& callback = onPreGroup[static_cast<uint32_t>(group)];

            if (callback)
            {
                callback(services, dt, group);
            }
        }

        void invokeSyncPoint(ServiceProvider& services, SystemGroup group, std::span<EntityChange const> entityChanges)
        {
            if (onSyncPoint)
            {
                onSyncPoint(services, group, entityChanges);
            }
        }
    };
}

#endif