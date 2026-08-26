#include "litl-core/authority.hpp"
#include "litl-core/hash.hpp"
#include "litl-core/thread.hpp"
#include "litl-engine/objects/material/deferredMaterialCommands.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/material/materialManager.hpp"

namespace litl
{
    std::unordered_map<MaterialHandle, std::vector<DeferredMaterialCommands::DeferredMaterialCommand>> DeferredMaterialCommands::s_combinedCommands;
    std::array<std::vector<DeferredMaterialCommands::DeferredMaterialCommand>, Constants::max_thread_count> DeferredMaterialCommands::t_threadCommands{};

    namespace
    {
        static std::vector<MaterialHandle> s_invalidMaterialHandles;
    }

    void DeferredMaterialCommands::onPreRender(Authority<MaterialManager> auth, ObjectPool& objectPool) noexcept
    {
        s_combinedCommands.clear();
        s_invalidMaterialHandles.clear();

        for (auto& threadCommands : t_threadCommands)
        {
            for (auto& threadCommand : threadCommands)
            {
                s_combinedCommands[threadCommand.handle].push_back(threadCommand);
            }

            threadCommands.clear();
        }

        for (auto& kvp : s_combinedCommands)
        {
            auto* material = objectPool.getMaterial(kvp.first);

            if (material != nullptr)
            {
                for (auto& command : kvp.second)
                {
                    switch (command.type)
                    {
                    case CommandType::MarkFrequentUpdates:
                        material->markAsFrequentUpdate(command.slot, true);
                        break;

                    case CommandType::MarkInfrequentUpdates:
                        material->markAsInfrequentUpdate(command.slot, true);
                        break;

                    case CommandType::Unknown:
                        break;
                    }
                }

                kvp.second.clear();
            }
            else
            {
                s_invalidMaterialHandles.push_back(kvp.first);
            }
        }

        for (auto invalidMaterialHandle : s_invalidMaterialHandles)
        {
            s_combinedCommands.erase(invalidMaterialHandle);
        }
    }

    void DeferredMaterialCommands::enqueue(CommandType commandType, MaterialPropertySlotId slot, MaterialHandle handle) noexcept
    {
        t_threadCommands[ThreadInfo::get().index].push_back(DeferredMaterialCommand{
            .type = commandType,
            .handle = handle,
            .slot = slot
        });
    }
}