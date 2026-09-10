#ifndef LITL_IMPORT_IMPORTED_DATA_H__
#define LITL_IMPORT_IMPORTED_DATA_H__

#include <concepts>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "litl-core/traits.hpp"
#include "litl-import/material/import/result.hpp"
#include "litl-import/mesh/import/result.hpp"
#include "litl-import/model/import/result.hpp"
#include "litl-import/shader/import/result.hpp"

namespace litl::import
{
    enum class ImportedDataType : uint32_t
    {
        Unknown = 0u,
        Material = 1u,
        Mesh = 2u,
        Model = 3u,
        Shader = 4u
    };

    /// <summary>
    /// The types of data that can be stored in ImportedData.
    /// </summary>
    template<typename T>
    concept ImportedDataTypeClass = IsAnyOf<
        T, 
        MaterialImportResult,
        MeshImportResult, 
        ModelImportResult,
        ShaderImportResult>;

    /// <summary>
    /// The unique pointer types used in ImportedData.
    /// </summary>
    using ImportedDataPtr = std::variant<           // The indices into this variant MUST MATCH the corresponding integer value of ImportedDataType.
        std::monostate,                             // maps to ImportedDataType::Unknown
        std::unique_ptr<MaterialImportResult>,      // maps to ImportedDataType::Material
        std::unique_ptr<MeshImportResult>,          // maps to ImportedDataType::Mesh
        std::unique_ptr<ModelImportResult>,         // maps to ImportedDataType::Model
        std::unique_ptr<ShaderImportResult>>;       // maps to ImportedDataType::Shader

    class ImportedDataItem
    {
    public:

        /// <summary>
        /// Returns the stored data type, if any.
        /// </summary>
        [[nodiscard]] ImportedDataType getType() const noexcept
        {
            return static_cast<ImportedDataType>(m_dataPtr.index());
        }

        /// <summary>
        /// Sets the data type stored in this import result.
        /// Note that once a type is set, it can not be changed.
        /// </summary>
        [[nodiscard]] bool setType(ImportedDataType type)
        {
            if (getType() == type)
            {
                return false;
            }

            if (getType() != ImportedDataType::Unknown)
            {
                logWarning("Attempting to override already-set ImportedData type. Once a type is set, it can not be undone.");
                return false;
            }

            switch (type)
            {
            case ImportedDataType::Material:
                m_dataPtr = std::make_unique<MaterialImportResult>();
                return true;

            case ImportedDataType::Mesh:
                m_dataPtr = std::make_unique<MeshImportResult>();
                return true;

            case ImportedDataType::Model:
                m_dataPtr = std::make_unique<ModelImportResult>();
                return true;

            case ImportedDataType::Shader:
                m_dataPtr = std::make_unique<ShaderImportResult>();
                return true;

            case ImportedDataType::Unknown:
            default:
                return false;
            }
        }

        /// <summary>
        /// Retrieves the pointer to the stored data in the specified form.
        /// If there is no stored data, or the wrong form is supplied, then returns null.
        /// </summary>
        template<ImportedDataTypeClass T>
        [[nodiscard]] T const* getDataPtr() const noexcept
        {
            auto* uniquePtr = std::get_if<std::unique_ptr<T>>(&m_dataPtr);

            if (uniquePtr != nullptr)
            {
                return uniquePtr->get();
            }

            return nullptr;
        }

        /// <summary>
        /// Retrieves the pointer to the stored data in the specified form.
        /// If there is no stored data, or the wrong form is supplied, then returns null.
        /// </summary>
        template<ImportedDataTypeClass T>
        [[nodiscard]] T* getDataPtr() noexcept
        {
            auto* uniquePtr = std::get_if<std::unique_ptr<T>>(&m_dataPtr);

            if (uniquePtr != nullptr)
            {
                return uniquePtr->get();
            }

            return nullptr;
        }

        void setName(std::string_view name) noexcept
        {
            m_name = name;
        }

        [[nodiscard]] std::string_view getName() const noexcept
        {
            return m_name;
        }

    private:

        ImportedDataPtr m_dataPtr;
        std::string m_name;
    };

    struct ImportedDataResult
    {
        /// <summary>
        /// The first non-success result response. This is the result for the item at the index specified in errorIndex.
        /// </summary>
        Result firstNonSuccessResult{};

        /// <summary>
        /// If an item failed to be processed, this is its index in the items vector.
        /// </summary>
        uint32_t errorIndex{ Constants::uint32_null_index };

        /// <summary>
        /// True if all items were properly processed.
        /// </summary>
        bool allSuccess{ true };
    };

    struct ImportedData
    {
        std::vector<ImportedDataItem> items;
        ImportedDataResult result{};

        void calculateTypeCounts() noexcept
        {
            m_dataTypeCounts.clear();

            for (auto& item : items)
            {
                m_dataTypeCounts[item.getType()]++;
            }
        }

        [[nodiscard]] uint32_t getTypeCount(ImportedDataType type) const noexcept
        {
            auto find = m_dataTypeCounts.find(type);

            if (find == m_dataTypeCounts.end())
            {
                return 0u;
            }

            return find->second;
        }

    private:


        std::unordered_map<ImportedDataType, uint32_t> m_dataTypeCounts;
    };
}

#endif