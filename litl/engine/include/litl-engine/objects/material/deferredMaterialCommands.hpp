#ifndef LITL_ENGINE_OBJECTS_MATERIAL_DEFERRED_COMMANDS_H__
#define LITL_ENGINE_OBJECTS_MATERIAL_DEFERRED_COMMANDS_H__

#include <array>
#include <unordered_map>
#include <variant>
#include <vector>

#include "litl-core/authority.hpp"
#include "litl-core/constants.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/material/materialPropertySlotId.hpp"
#include "litl-engine/objects/material/materialBindings.hpp"
#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    class MaterialManager;
    class ObjectPool;
    struct Entity;

    enum class DeferredMaterialCommandType : uint32_t
    {
        Unknown = 0u,
        UpgradeSlotToFrequentBlock = 1u
    };

    struct UpdateSlotToFrequentBlockCommand
    {
        static constexpr DeferredMaterialCommandType Type = DeferredMaterialCommandType::UpgradeSlotToFrequentBlock;
        MaterialHandle handle{};
        MaterialPropertySlotId slot{};
    };

    /// <summary>
    /// Takes in thread-safe deferred material-related commands from ECS systems and then processes them on the main thread during onPreRender.
    /// </summary>
    class DeferredMaterialCommands
    {
    public:

        static void onPreRender(Authority<MaterialManager> auth, ObjectPool& objectPool) noexcept;
        static void enqueueUpgradeSlotCommand(UpdateSlotToFrequentBlockCommand const& command) noexcept;

    private:

        struct DeferredMaterialCommand
        {
            DeferredMaterialCommandType type{ DeferredMaterialCommandType::Unknown };
            std::variant<UpdateSlotToFrequentBlockCommand> command;
        };

        static std::array<std::vector<DeferredMaterialCommand>, Constants::max_thread_count> t_threadCommands;
    };
}

#endif