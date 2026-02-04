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

        ~PipelineLayout() = default;

        virtual bool rebuild(PipelineLayoutDescriptor const& descriptor) noexcept = 0;

        PipelineLayoutDescriptor const& getDescriptor() const noexcept
        {
            return m_descriptor;
        }

    protected:

        PipelineLayoutDescriptor m_descriptor;

    private:
    };
}

#endif