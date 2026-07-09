#ifndef LITL_ENGINE_OBJECTS_CAMERA_DESCRIPTOR_H__
#define LITL_ENGINE_OBJECTS_CAMERA_DESCRIPTOR_H__

namespace litl
{
    /// <summary>
    /// Predefined selection of camera processing order values.
    /// Lower values are processed first.
    /// </summary>
    enum class CameraProcessOrder : uint32_t
    {
        /// <summary>
        /// Use for cameras that are to be rendered/processed before the main camera.
        /// </summary>
        Preprocess = 50u,

        /// <summary>
        /// The process position enforced for main camera.
        /// </summary>
        MainCamera = 100u,

        /// <summary>
        /// Use for cameras that are to be rendered/processed after the main camera.
        /// </summary>
        Postprocess = 150u
    };
    struct CameraDescriptor
    {
        /// <summary>
        /// The priority level for the camera.
        /// Cameras with a lower level are processed first.
        /// </summary>
        uint32_t processOrder = static_cast<uint32_t>(CameraProcessOrder::Postprocess);
    };
}

#endif