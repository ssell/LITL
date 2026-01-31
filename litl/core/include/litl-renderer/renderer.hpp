#ifndef LITL_CORE_RENDERER_H__
#define LITL_CORE_RENDERER_H__

#include <cstdint>
#include "litl-renderer/rendererDescriptor.hpp"

namespace LITL::Renderer
{
    class Renderer
    {
    public:

        virtual ~Renderer() = default;

        virtual bool initialize() const noexcept = 0;

    protected:
        
    private:
    };
}

#endif