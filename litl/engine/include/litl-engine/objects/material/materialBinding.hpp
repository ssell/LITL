#ifndef LITL_ENGINE_OBJECTS_MATERIAL_BINDING_H__
#define LITL_ENGINE_OBJECTS_MATERIAL_BINDING_H__

#include <optional>
#include <vector>

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/material/materialPropertySlotId.hpp"

namespace litl
{
    class ObjectPool;
    class Material;

    /// <summary>
    /// An individual material -> submesh binding.
    /// </summary>
    struct MaterialBinding
    {
        /// <summary>
        /// The versioned handle to the shared Material object.
        /// This is used in conjunction with the ObjectPool to retrieve a pointer to the Material.
        /// </summary>
        MaterialHandle handle{};

        /// <summary>
        /// The slot that this instance occupies in the shared Material object.
        /// A slot is obtained by calling Material::allocateSlot.
        /// </summary>
        MaterialPropertySlotId slot{};
    };

    struct MaterialBindingsDescriptor
    {
        ObjectDescriptor objectInfo{};
        std::vector<MaterialBinding> bindings;
    };

    /// <summary>
    /// One or more material bindings for a renderable entity.
    /// 
    /// If there is only one binding, then that material is applied to the entire mesh.
    /// If there are more than one bindings, then each binding slot corresponds with a submesh.
    /// If there are more submeshes than materials, then only those submeshes with a bound material will be rendered.
    /// 
    /// It is expected that if there are multiple submeshes then each has a designated material binding. The excess
    /// submeshes are intentionally not rendered to serve as a visual signal that there is a misconfiguration.
    /// </summary>
    class MaterialBindings
    {
    public:

        [[nodiscard]] bool create(Authority<ObjectPool> auth, ObjectPool& pool, MaterialBindingsDescriptor const& descriptor) noexcept;
        void destroy(Authority<ObjectPool> auth) noexcept;

        /// <summary>
        /// Returns the number of bindings.
        /// </summary>
        [[nodiscard]] uint32_t getBindingsCount() const noexcept;

        /// <summary>
        /// Retrieves the binding at the specified index.
        /// If the index is not valid, then will return std::nullopt.
        /// </summary>
        [[nodiscard]] std::optional<MaterialBinding> getBinding(uint32_t index) const noexcept;

        /// <summary>
        /// Retrieves the material bound at the specified index.
        /// If the index is not valid, then will return null.
        /// </summary>
        [[nodiscard]] Material* getBoundMaterial(uint32_t index) const noexcept;

        /// <summary>
        /// Sets the binding at the specified index.
        /// If the index is not valid, then will return false.
        /// </summary>
        bool setBinding(uint32_t index, MaterialBinding binding) noexcept;

    private:

        ObjectPool* m_pObjectPool{ nullptr };
        std::vector<MaterialBinding> m_bindings;
    };
}

#endif