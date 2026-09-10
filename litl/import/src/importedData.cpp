#include "litl-import/importedData.hpp"

namespace litl::import
{
    ImportedDataType ImportedDataItem::getType() const noexcept
    {
        return static_cast<ImportedDataType>(m_dataPtr.index());
    }

    bool ImportedDataItem::setType(ImportedDataType type) noexcept
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

    void ImportedDataItem::setName(std::string_view name) noexcept
    {
        m_name = name;
    }

    std::string_view ImportedDataItem::getName() const noexcept
    {
        return m_name;
    }

    void ImportedData::calculateTypeCounts() noexcept
    {
        m_dataTypeCounts.clear();

        for (auto& item : items)
        {
            m_dataTypeCounts[item.getType()]++;
        }
    }

    uint32_t ImportedData::getTypeCount(ImportedDataType type) const noexcept
    {
        auto find = m_dataTypeCounts.find(type);

        if (find == m_dataTypeCounts.end())
        {
            return 0u;
        }

        return find->second;
    }

    void ImportedData::propagateNameUpdates() noexcept
    {
        for (auto& dataItem : items)
        {
            if (dataItem.getType() == ImportedDataType::Model)
            {
                auto* modelPtr = dataItem.getDataPtr<ModelImportResult>();

                if ((modelPtr != nullptr) && !modelPtr->dataItems.empty())
                {
                    for (auto& modelItem : modelPtr->dataItems)
                    {
                        if (modelItem.importedDataItemIndex >= items.size())
                        {
                            continue;
                        }

                        auto& modelDataItem = items[modelItem.importedDataItemIndex];

                        if (modelItem.modelNameIndex != Constants::uint32_null_index)
                        {
                            switch (modelDataItem.getType())
                            {
                            case ImportedDataType::Mesh:
                                modelPtr->model->updateMeshName(modelItem.modelNameIndex, modelDataItem.getName());
                                break;

                            // TODO
                            //case ImportedDataType::Material:
                            //    modelPtr->model->updateMaterialName(modelItem.modelNameIndex, modelDataItem.getName());
                            //    break;

                            default:
                                break;
                            }
                        }

                        if (modelItem.modelNodeIndex != Constants::uint32_null_index)
                        {
                            modelPtr->model->updateNodeName(modelItem.modelNodeIndex, modelDataItem.getName());
                        }
                    }
                }
            }
        }
    }
}