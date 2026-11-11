#include <cstring>
#include <span>

#include "litl-renderer/resources/specializationConstants.hpp"
#include "litl-core/logging/logging.hpp"

namespace litl
{
    namespace
    {
        [[nodiscard]] std::optional<size_t> alreadyExists(uint32_t id, std::span<SpecializationConstantEntry const> entries) noexcept
        {
            for (size_t i = 0ull; i < entries.size() ; ++i)
            {
                if (entries[i].id == id)
                {
                    return i;
                }
            }

            return std::nullopt;
        }
    }

    void SpecializationConstants::addEntry(uint32_t id, bool value) noexcept
    {
        SpecializationConstantEntry entry{ .id = id, .size = 4u };
        const uint32_t bool4Bytes = (value ? 1u : 0u);
        std::memcpy(entry.data.data(), &bool4Bytes, 4ull);

        const auto index = alreadyExists(id, entries);

        if (index.has_value())
        {
            entries[index.value()] = entry;
        }
        else
        {
            entries.push_back(entry);
        }
    }

    void SpecializationConstants::addEntry(uint32_t id, int32_t value) noexcept
    {
        SpecializationConstantEntry entry{ .id = id, .size = 4u };
        std::memcpy(entry.data.data(), &value, 4ull);

        const auto index = alreadyExists(id, entries);

        if (index.has_value())
        {
            entries[index.value()] = entry;
        }
        else
        {
            entries.push_back(entry);
        }
    }

    void SpecializationConstants::addEntry(uint32_t id, uint32_t value) noexcept
    {
        SpecializationConstantEntry entry{ .id = id, .size = 4u };
        std::memcpy(entry.data.data(), &value, 4ull);

        const auto index = alreadyExists(id, entries);

        if (index.has_value())
        {
            entries[index.value()] = entry;
        }
        else
        {
            entries.push_back(entry);
        }
    }

    void SpecializationConstants::addEntry(uint32_t id, float value) noexcept
    {
        SpecializationConstantEntry entry{ .id = id, .size = 4u };
        std::memcpy(entry.data.data(), &value, 4ull);

        const auto index = alreadyExists(id, entries);

        if (index.has_value())
        {
            entries[index.value()] = entry;
        }
        else
        {
            entries.push_back(entry);
        }
    }

    void SpecializationConstants::addEntry(uint32_t id, double value) noexcept
    {
        SpecializationConstantEntry entry{ .id = id, .size = 4u };
        std::memcpy(entry.data.data(), &value, 8ull);

        const auto index = alreadyExists(id, entries);

        if (index.has_value())
        {
            entries[index.value()] = entry;
        }
        else
        {
            entries.push_back(entry);
        }
    }
}