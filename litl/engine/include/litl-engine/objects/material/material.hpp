#ifndef LITL_ENGINE_MATERIAL_H__
#define LITL_ENGINE_MATERIAL_H__

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "litl-core/authority.hpp"
#include "litl-core/stringId.hpp"
#include "litl-core/math/geometry/vertex.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/material/materialPropertySlotId.hpp"
#include "litl-renderer/resources/graphicsPipeline.hpp"
#include "litl-renderer/resources/computePipeline.hpp"

namespace litl
{
    class MaterialManager;
    class ObjectPool;
    class Renderer;

    struct VertexInputDescriptor
    {
        static constexpr uint32_t MaxVertexAttributes = 8u;

        /// <summary>
        /// Size of the vertex structure.
        /// </summary>
        uint32_t vertexSize = sizeof(Vertex);

        /// <summary>
        /// The data format of each field of the vertex structure.
        /// </summary>
        std::array<DataFormat, MaxVertexAttributes> attributes = {
            DataFormat::RGB32_SFloat,                   // position
            DataFormat::RG32_SFloat,                    // texcoord
            DataFormat::RGB32_SFloat,                   // normal
            DataFormat::RGBA32_SFloat                   // tangent
        };
    };

    struct ShaderResourceDescriptor
    {
        std::string resource{};
        std::string entryPoint{};
        std::span<std::byte const> bytes{};
    };

    struct MaterialDescriptor
    {
        ObjectDescriptor objectInfo{};
        RasterizationState rasterizerState{};
        VertexInputDescriptor inputDescriptor{};
        ShaderResourceDescriptor vertexShader{};
        ShaderResourceDescriptor fragmentShader{};
        ShaderResourceDescriptor geometryShader{};
        ShaderResourceDescriptor tessellationControlShader{};
        ShaderResourceDescriptor tessellationEvaluationShader{};
        ShaderResourceDescriptor computeShader{};
        ShaderResourceDescriptor meshShader{};
        ShaderResourceDescriptor taskShader{};
    };

    class Material
    {
    public:

        /// <summary>
        /// The required name of the material properties variable in the PushConstant struct.
        /// </summary>
        static constexpr std::string_view MaterialPropertiesStructName = "materialProperties";
        static constexpr StringId MaterialPropertiesStructHashedName = StringId(MaterialPropertiesStructName);

        Material();
        Material(Material&& other) noexcept;
        Material& operator=(Material&& other) noexcept;
        ~Material();

        bool create(Authority<ObjectPool> auth, MaterialDescriptor const& descriptor, Renderer const& renderer, ObjectPool& objectPool) noexcept;
        void destroy(Authority<ObjectPool> auth) noexcept;

        [[nodiscard]] GraphicsPipelineHandle getGraphicsPipelineHandle() const noexcept;
        [[nodiscard]] ComputePipelineHandle getComputePipelineHandle() const noexcept;
        [[nodiscard]] GpuBufferHandle getGraphicsGpuBufferHandle() const noexcept;
        [[nodiscard]] std::optional<uint64_t> getGraphicsBufferDeviceAddress() const noexcept;
        [[nodiscard]] MaterialPropertySlotId allocateSlot() noexcept;

        void onFrameStart(Authority<MaterialManager> auth, uint32_t frame, uint32_t frameInFlightIndex) noexcept;
        void onPreRender(Authority<MaterialManager> auth) noexcept;
        void markActive(MaterialPropertySlotId slot) noexcept;

        [[nodiscard]] bool ready() const noexcept;

        /// <summary>
        /// Sets the boolean value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setBool(StringId property, bool value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 32-bit signed integer value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setInt32(StringId property, int32_t value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 32-bit unsigned integer value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setUint32(StringId property, uint32_t value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 32-bit float value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setFloat(StringId property, float value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 64-bit float value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setDouble(StringId property, double value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the two-component 32-bit float vector value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setVec2(StringId property, vec2 value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the three-component 32-bit float vector value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setVec3(StringId property, vec3 value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the four-component 32-bit float vector value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setVec4(StringId property, vec4 const& value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the three- or four-component 32-bit float vector value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setColor(StringId property, color const& value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 3x3 32-bit float matrix value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setMat3(StringId property, mat3 const& value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 4x4 32-bit float matrix value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setMat4(StringId property, mat4 const& value, MaterialPropertySlotId slot) noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif