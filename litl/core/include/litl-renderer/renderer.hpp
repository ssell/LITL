#ifndef LITL_CORE_RENDERER_H__
#define LITL_CORE_RENDERER_H__

#include <cstdint>

#include "litl-renderer/rendererDescriptor.hpp"
#include "litl-renderer/commands/commandBuffer.hpp"
#include "litl-renderer/pipeline/pipelineLayout.hpp"

namespace LITL::Renderer
{
    class Renderer
    {
    public:

        virtual ~Renderer() = default;
        virtual bool initialize() noexcept = 0;

        virtual uint32_t getFrame() const noexcept = 0;
        virtual uint32_t getFrameIndex() const noexcept = 0;

        virtual std::unique_ptr<CommandBuffer> createCommandBuffer() const noexcept = 0;

        virtual void render(CommandBuffer* pCommandBuffers, uint32_t numCommandBuffers) = 0;

    protected:
        
    private:
    };
}

#endif