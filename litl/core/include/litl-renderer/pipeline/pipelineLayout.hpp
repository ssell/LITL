#ifndef LITL_RENDERER_PIPELINE_LAYOUT_H__
#define LITL_RENDERER_PIPELINE_LAYOUT_H__

namespace LITL::Renderer
{
    struct PipelineLayoutDescriptor
    {
        // ... todo ...
    };

    class PipelineLayout
    {
    public:

        PipelineLayoutDescriptor const& getDescriptor()
        {
            return m_descriptor;
        }

    protected:

    private:

        PipelineLayoutDescriptor m_descriptor;
    };
}

#endif