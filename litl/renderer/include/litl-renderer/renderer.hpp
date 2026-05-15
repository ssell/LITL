#ifndef LITL_RENDERER_H__
#define LITL_RENDERER_H__

#include "litl-core/assert.hpp"
#include "litl-renderer/rendererConfiguration.hpp"

namespace litl
{
    struct RendererContext;

    struct RendererOps
    {
        bool (*build)(RendererContext*);
        void (*destroy)(RendererContext*);
    };

    /// <summary>
    /// 
    /// </summary>
    class Renderer final
    {
    public:

        /// <summary>
        /// Provided only for initial service injection.
        /// </summary>
        Renderer() = default;

        Renderer(RendererContext* context, RendererOps const* ops)
            : m_pContext(context), m_pOps(ops)
        {

        }

        bool build()
        {
            LITL_FATAL_ASSERT_MSG(valid(), "Renderer::build called with invalid internal state");
            return m_pOps->build(m_pContext);
        }

        void destroy()
        {
            LITL_FATAL_ASSERT_MSG(valid(), "Renderer::destroy called with invalid internal state");

            m_pOps->destroy(m_pContext);
            m_pContext = nullptr;
            m_pOps = nullptr;
        }

        bool beginRender() { return false; };
        void endRender() {}

    private:

        [[nodiscard]] bool valid() const noexcept
        {
            return (m_pContext != nullptr) && (m_pOps != nullptr);
        }

        RendererContext* m_pContext;
        RendererOps const* m_pOps;
    };
}

#endif