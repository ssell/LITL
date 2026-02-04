#ifndef LITL_CORE_RENDERER_H__
#define LITL_CORE_RENDERER_H__

#include <cstdint>

#include "litl-renderer/rendererDescriptor.hpp"
#include "litl-renderer/commands/commandBuffer.hpp"

namespace LITL::Renderer
{
    class Renderer
    {
    public:

        virtual ~Renderer() = default;

        virtual bool initialize() const noexcept = 0;

        /// <summary>
        /// Creates a new command buffer for the active backend. If successful, the caller assumes ownership (and responsibilil
        /// </summary>
        /// <returns></returns>
        virtual std::unique_ptr<CommandBuffer> createCommandBuffer() const noexcept = 0;

    protected:
        
    private:
    };
}

#endif