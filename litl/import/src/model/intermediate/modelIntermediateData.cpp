#include "litl-import/model/intermediate/modelIntermediateData.hpp"

namespace litl::import
{
    void ModelIntermediateData::setName(std::string_view name) noexcept
    {
        m_name = name;
    }

    uint32_t ModelIntermediateData::addMesh(std::string_view meshName) noexcept
    {
        const uint32_t index = static_cast<uint32_t>(m_meshNames.size());
        m_meshNames.push_back(std::string(meshName));
        return index;
    }

    uint32_t ModelIntermediateData::addMaterial(std::string_view materialName) noexcept
    {
        const uint32_t index = static_cast<uint32_t>(m_materialNames.size());
        m_materialNames.push_back(std::string(materialName));
        return index;
    }

    uint32_t ModelIntermediateData::addNode(Node&& node) noexcept
    {
        const uint32_t index = static_cast<uint32_t>(m_nodes.size());
        m_nodes.push_back(std::move(node));
        return index;
    }

    void ModelIntermediateData::addRootNode(uint32_t nodeIndex) noexcept
    {
        m_rootNodes.push_back(nodeIndex);
    }

    void ModelIntermediateData::updateMeshName(uint32_t meshIndex, std::string_view updatedName) noexcept
    {
        if (meshIndex >= m_meshNames.size())
        {
            return;
        }

        m_meshNames[meshIndex] = updatedName;
    }

    void ModelIntermediateData::updateMaterialName(uint32_t materialIndex, std::string_view updatedName) noexcept
    {
        if (materialIndex >= m_materialNames.size())
        {
            return;
        }

        m_materialNames[materialIndex] = updatedName;
    }

    std::string_view ModelIntermediateData::getName() const noexcept
    {
        return m_name;
    }

    std::span<std::string const> ModelIntermediateData::getMeshNames() const noexcept
    {
        return m_meshNames;
    }

    std::span<std::string const> ModelIntermediateData::getMaterialNames() const noexcept
    {
        return m_materialNames;
    }

    std::span<Node const> ModelIntermediateData::getNodes() const noexcept
    {
        return m_nodes;
    }

    std::span<uint32_t const> ModelIntermediateData::getRootNodes() const noexcept
    {
        return m_rootNodes;
    }
}
