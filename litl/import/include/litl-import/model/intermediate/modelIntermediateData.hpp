#ifndef LITL_IMPORT_MODEL_INTERMEDIATE_DATA_H__
#define LITL_IMPORT_MODEL_INTERMEDIATE_DATA_H__

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "litl-import/model/intermediate/node.hpp"

namespace litl::import
{
    class ModelIntermediateData
    {
    public:

        void setName(std::string_view name) noexcept;
        uint32_t addMesh(std::string_view meshName) noexcept;
        uint32_t addNode(Node&& node) noexcept;
        void addRootNode(uint32_t nodeIndex) noexcept;

        [[nodiscard]] std::string_view getName() const noexcept;
        [[nodiscard]] std::span<std::string const> getMeshNames() const noexcept;
        [[nodiscard]] std::span<Node const> getNodes() const noexcept;
        [[nodiscard]] std::span<uint32_t const> getRootNodes() const noexcept;

    private:

        std::string m_name;
        std::vector<std::string> m_meshNames;
        std::vector<Node> m_nodes;
        std::vector<uint32_t> m_rootNodes;
    };
}

#endif