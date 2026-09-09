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
    /// It is expected that if there are multiple submeshes then each has a designated material binding. The excess
    /// submeshes are intentionally not rendered to serve as a visual signal that there is a misconfiguration.
    /// 
    /// Each MaterialBindings instance is tied directly to a VariableMaterialRefs component.
    /// A MaterialBindings tied to a deleted VariableMaterialRefs component will eventually be garbage collected.
    /// </summary>
    class MaterialBindings
    {
    public:

        /// <summary>
        /// 
        /// </summary>
        [[nodiscard]] bool create(Authority<ObjectPool> auth, ObjectPool& pool, MaterialBindingsDescriptor const& descriptor) noexcept;

        /// <summary>
        /// 
        /// </summary>
        void destroy(Authority<ObjectPool> auth) noexcept;

        /// <summary>
        /// 
        /// </summary>
        void setSelfHandle(Authority<ObjectPool> author, MaterialBindingsHandle handle) noexcept;

        /// <summary>
        /// 
        /// </summary>
        [[nodiscard]] MaterialBindingsHandle getHandle() const noexcept;

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
        /// Retrieves all bindings.
        /// </summary>
        [[nodiscard]] std::vector<MaterialBinding> const& getBindings() const noexcept;

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

        void setLastActiveFrame(uint32_t currFrame) noexcept;

        [[nodiscard]] uint32_t getLastActiveFrame() const noexcept;

    private:

        ObjectPool* m_pObjectPool{ nullptr };
        MaterialBindingsHandle m_handle{};
        uint32_t m_lastActiveFrame{ 0u };
        std::vector<MaterialBinding> m_bindings;
    };
}

#endif