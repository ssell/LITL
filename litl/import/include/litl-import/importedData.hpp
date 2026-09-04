#ifndef LITL_IMPORT_IMPORTED_DATA_H__
#define LITL_IMPORT_IMPORTED_DATA_H__

#include <concepts>
#include <memory>
#include <variant>

#include "litl-core/traits.hpp"
#include "litl-import/material/import/result.hpp"
#include "litl-import/mesh/import/result.hpp"
#include "litl-import/shader/import/result.hpp"

namespace litl::import
{
    enum class ImportedDataType : uint32_t
    {
        Unknown = 0u,
        Material = 1u,
        Mesh = 2u,
        Shader = 3u
    };

    /// <summary>
    /// The types of data that can be stored in ImportedData.
    /// </summary>
    template<typename T>
    concept ImportedDataTypeClass = IsAnyOf<
        T, 
        MaterialImportResult,
        MeshImportResult, 
        ShaderImportResult>;

    /// <summary>
    /// The unique pointer types used in ImportedData.
    /// </summary>
    using ImportedDataPtr = std::variant<
        std::monostate,     // maps to ImportedDataType::Unknown
        std::unique_ptr<MaterialImportResult>,
        std::unique_ptr<MeshImportResult>,
        std::unique_ptr<ShaderImportResult>>;

    class ImportedData final
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
        void setType(ImportedDataType type)
        {
            if (m_type == type)
            {
                return;
            }

            if (m_type != ImportedDataType::Unknown)
            {
                logWarning("Attempting to override already-set ImportedData type. Once a type is set, it can not be undone.");
                return;
            }

            m_type = type;

            switch (m_type)
            {
            case ImportedDataType::Material:
                m_dataPtr = std::make_unique<MaterialImportResult>();
                break;

            case ImportedDataType::Mesh:
                m_dataPtr = std::make_unique<MeshImportResult>();
                break;

            case ImportedDataType::Shader:
                m_dataPtr = std::make_unique<ShaderImportResult>();
                break;

            case ImportedDataType::Unknown:
            default:
                break;
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


    private:

        ImportedDataType m_type{ ImportedDataType::Unknown };
        ImportedDataPtr m_dataPtr;
    };
}

#endif