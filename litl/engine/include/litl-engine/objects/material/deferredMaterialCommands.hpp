#ifndef LITL_ENGINE_OBJECTS_MATERIAL_DEFERRED_COMMANDS_H__
#define LITL_ENGINE_OBJECTS_MATERIAL_DEFERRED_COMMANDS_H__

#include <array>
#include <unordered_map>
#include <vector>

#include "litl-core/constants.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/material/materialPropertySlotId.hpp"

namespace litl
{
    class MaterialManager;
    class ObjectPool;

    class DeferredMaterialCommands
    {
    public:

        enum class CommandType : uint32_t
        {
            Unknown = 0u,
            MarkFrequentUpdates = 1u,
            MarkInfrequentUpdates = 2u
        };

        static void onPreRender(Authority<MaterialManager> auth, ObjectPool& objectPool) noexcept;
        static void enqueue(CommandType commandType, MaterialPropertySlotId slot, MaterialHandle handle) noexcept;

    private:

        struct DeferredMaterialCommand
        {
            CommandType type{ CommandType::Unknown };
            MaterialHandle handle{};
            MaterialPropertySlotId slot{};
        };

        static std::unordered_map<MaterialHandle, std::vector<DeferredMaterialCommand>> s_combinedCommands;
        static std::array<std::vector<DeferredMaterialCommand>, Constants::max_thread_count> t_threadCommands;
    };
}

#endif