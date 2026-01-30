#ifndef LITL_CORE_RENDERER_H__
#define LITL_CORE_RENDERER_H__

#include <cstdint>

namespace LITL::Renderer
{
    class Renderer
    {
    public:

        virtual bool initialize(const char* title, uint32_t width, uint32_t height) const noexcept = 0;

    protected:
        
    private:
    };
}

#endif