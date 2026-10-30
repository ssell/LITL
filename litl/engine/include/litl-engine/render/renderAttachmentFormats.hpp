#ifndef LITL_ENGINE_RENDER_ATTACHMENT_FORMATS_H__
#define LITL_ENGINE_RENDER_ATTACHMENT_FORMATS_H__

#include "litl-core/formats/dataFormats.hpp"

namespace litl
{
    /// <summary>
    /// The data format used by the scene, which may differ from the format of the swapchain.
    /// For example, for HDR lighting the scene format may be RGBA16_SFloat while the swapchain is still RGBA8_UNorm.
    /// </summary>
    struct RenderAttachmentFormats
    {
        DataFormat color{ DataFormat::Undefined };
        DataFormat depth{ DataFormat::Undefined };
    };
}

#endif