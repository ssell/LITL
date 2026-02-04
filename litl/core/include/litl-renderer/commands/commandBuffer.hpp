#ifndef LITL_RENDERER_COMMAND_BUFFER_H__
#define LITL_RENDERER_COMMAND_BUFFER_H__

namespace LITL::Renderer
{
    class CommandBuffer
    {
    public:

        virtual ~CommandBuffer() = default;

        virtual bool build() = 0;
        virtual bool begin(uint32_t frame) = 0;
        virtual bool end() = 0;

    protected:

        uint32_t m_currFrame;

    private:
    };
}

#endif