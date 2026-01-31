#include "litl-renderer-vulkan/renderer.hpp"
#include "litl-renderer-vulkan/renderContext.hpp"

namespace LITL::Vulkan::Renderer
{
    struct Renderer::Impl
    {
        RenderContext* pContext;

        ~Impl()
        {
            if (pContext != nullptr)
            {
                delete pContext;
            }
        }
    };

    LITL::Renderer::Renderer* createVulkanRenderer(LITL::Renderer::RendererDescriptor const& rendererDescriptor)
    {
        return static_cast<LITL::Renderer::Renderer*>(new Renderer(rendererDescriptor));
    }

    Renderer::Renderer(LITL::Renderer::RendererDescriptor const& rendererDescriptor)
    {
        m_impl->pContext = new RenderContext{
            .framesInFlight = rendererDescriptor.framesInFlight
        };
    }

    Renderer::~Renderer()
    {

    }

    bool Renderer::initialize() const noexcept
    {
        return true;
    }
}