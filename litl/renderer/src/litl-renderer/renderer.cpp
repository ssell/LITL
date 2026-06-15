#include "litl-renderer/renderer.hpp"
#include "litl-renderer/reflection.hpp"

namespace litl
{
    PipelineResourceKey Renderer::getPipelineResourceKey(std::string_view name) const noexcept
    {
        return PipelineResourceMap::getKey(name);
    }
}