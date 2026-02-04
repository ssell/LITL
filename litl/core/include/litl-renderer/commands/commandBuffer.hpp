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
        virtual bool end(uint32_t frame) = 0;

    protected:

    private:
    };
}

#endif