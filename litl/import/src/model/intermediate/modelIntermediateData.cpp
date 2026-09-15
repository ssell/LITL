#include "litl-import/model/intermediate/modelIntermediateData.hpp"

namespace litl::import
{
    void ModelIntermediateData::clear() noexcept
    {
        m_model = {};
    }

    void ModelIntermediateData::setName(std::string_view name) noexcept
    {
        m_model.name = name;
    }

    uint32_t ModelIntermediateData::addMesh(std::string_view meshName) noexcept
    {
        const uint32_t index = static_cast<uint32_t>(m_model.meshNames.size());
        m_model.meshNames.push_back(std::string(meshName));
        return index;
    }

    uint32_t ModelIntermediateData::addMaterial(std::string_view materialName) noexcept
    {
        const uint32_t index = static_cast<uint32_t>(m_model.materialNames.size());
        m_model.materialNames.push_back(std::string(materialName));
        return index;
    }

    uint32_t ModelIntermediateData::addNode(Node&& node) noexcept
    {
        const uint32_t index = static_cast<uint32_t>(m_model.nodes.size());
        m_model.nodes.push_back(std::move(node));
        return index;
    }

    void ModelIntermediateData::addRootNode(uint32_t nodeIndex) noexcept
    {
        m_model.rootNodes.push_back(nodeIndex);
    }

    void ModelIntermediateData::updateMeshName(uint32_t meshIndex, std::string_view updatedName) noexcept
    {
        if (meshIndex >= m_model.meshNames.size())
        {
            return;
        }

        m_model.meshNames[meshIndex] = updatedName;
    }

    void ModelIntermediateData::updateMaterialName(uint32_t materialIndex, std::string_view updatedName) noexcept
    {
        if (materialIndex >= m_model.materialNames.size())
        {
            return;
        }

        m_model.materialNames[materialIndex] = updatedName;
    }

    std::string_view ModelIntermediateData::getName() const noexcept
    {
        return m_model.name;
    }

    std::span<std::string const> ModelIntermediateData::getMeshNames() const noexcept
    {
        return m_model.meshNames;
    }

    std::span<std::string const> ModelIntermediateData::getMaterialNames() const noexcept
    {
        return m_model.materialNames;
    }

    std::span<Node const> ModelIntermediateData::getNodes() const noexcept
    {
        return m_model.nodes;
    }

    std::span<uint32_t const> ModelIntermediateData::getRootNodes() const noexcept
    {
        return m_model.rootNodes;
    }

    Model const& ModelIntermediateData::getModel() const noexcept
    {
        return m_model;
    }

    Model& ModelIntermediateData::getModelWriteRef() noexcept
    {
        return m_model;
    }
}
