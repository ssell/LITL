#ifndef LITL_ECS_FRAME_CALLBACKS_H__
#define LITL_ECS_FRAME_CALLBACKS_H__

#include <array>
#include <span>
#include <vector>

#include "litl-core/inlineFunc.hpp"
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

        inline_function<void()> onFrameStart{ nullptr };
        inline_function<void()> onFrameEnd{ nullptr };
        inline_function<void(SystemGroup, std::span<EntityChange const>)> onSyncPoint{ nullptr };

        std::array<inline_function<void(SystemGroup)>, GroupCount> onPreGroup{ nullptr };
        std::array<inline_function<void(SystemGroup)>, GroupCount> onPostGroup{ nullptr };

        void invokeFrameStart() const noexcept
        {
            if (onFrameStart)
            {
                onFrameStart();
            }
        }

        void invokeFrameEnd() const noexcept
        {
            if (onFrameEnd)
            {
                onFrameEnd();
            }
        }

        void invokePreGroup(SystemGroup group) const noexcept
        {
            auto& callback = onPreGroup[static_cast<uint32_t>(group)];

            if (callback)
            {
                callback(group);
            }
        }

        void invokeSyncPoint(SystemGroup group, std::span<EntityChange const> entityChanges)
        {
            if (onSyncPoint)
            {
                onSyncPoint(group, entityChanges);
            }
        }

        void invokePostGroup(SystemGroup group) const noexcept
        {
            auto& callback = onPostGroup[static_cast<uint32_t>(group)];

            if (callback)
            {
                callback(group);
            }
        }
    };
}

#endif