#include <memory>
#include <mutex>
#include <vector>

#include "litl-engine/ecs/system/systemRegistry.hpp"

namespace LITL::Engine::ECS
{
    struct SystemRegistryState
    {
        SystemRegistryState()
        {
            systemRecords.reserve(128);
        }

        std::mutex systemRecordsMutex;
        std::vector<SystemRecord const*> systemRecords;
    };

    namespace
    {
        static SystemRegistryState& instance() noexcept
        {
            static SystemRegistryState registry;
            return registry;
        }
    }

    SystemTypeId SystemRecord::nextId() noexcept
    {
        static SystemTypeId id = 1;
        return id++;
    }

    SystemTypeId SystemRecord::track(SystemRecord const* record) noexcept
    {
        auto& registry = instance();

        std::lock_guard<std::mutex> lock(registry.systemRecordsMutex);
        const SystemTypeId id = registry.systemRecords.size();

        registry.systemRecords.push_back(record);
        
        return id;
    }
    /*
    SystemRecord const& SystemRegistry::get(SystemTypeId systemTypeId)
    {
        auto& registry = instance();
        return *registry.systemRecords[systemTypeId];
    }
    */
}