#include "litl-core/authority.hpp"
#include "litl-core/thread.hpp"
#include "litl-engine/objects/material/deferredMaterialCommands.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/material/materialManager.hpp"

namespace litl
{
    std::array<std::vector<DeferredMaterialCommands::DeferredMaterialCommand>, Constants::max_thread_count> DeferredMaterialCommands::t_threadCommands{};

    namespace
    {
        static std::vector<MaterialHandle> s_invalidMaterialHandles;
    }

    void DeferredMaterialCommands::onPreRender(Authority<MaterialManager> auth, ObjectPool& objectPool) noexcept
    {
        for (auto& threadCommands : t_threadCommands)
        {
            for (auto& command : threadCommands)
            {
                switch (command.type)
                {
                case DeferredMaterialCommandType::UpgradeSlotToFrequentBlock:
                    {
                        const auto* pCommand = std::get_if<UpdateSlotToFrequentBlockCommand>(&command.command);

                        if (pCommand != nullptr)
                        {
                            auto* material = objectPool.getMaterial(pCommand->handle);

                            if (material != nullptr)
                            {
                                material->upgradeSlotToFrequentBlock({}, pCommand->slot);
                            }
                        }
                    }
                    break;

                default:
                    break;
                }
            }

            threadCommands.clear();
        }
    }

    void DeferredMaterialCommands::enqueueUpgradeSlotCommand(UpdateSlotToFrequentBlockCommand const& command) noexcept
    {
        t_threadCommands[ThreadInfo::get().index].push_back(DeferredMaterialCommand{
            .type = UpdateSlotToFrequentBlockCommand::Type,
            .command = command
        });
    }
}