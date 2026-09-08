#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/material/materialBinding.hpp"
#include "litl-engine/objects/material/material.hpp"

namespace litl
{
    bool MaterialBindings::create(Authority<ObjectPool> auth, ObjectPool& objectPool, MaterialBindingsDescriptor const& descriptor) noexcept
    {
        m_pObjectPool = &objectPool;
        m_bindings.assign(descriptor.bindings.begin(), descriptor.bindings.end());

        return true;
    }

    void MaterialBindings::destroy(Authority<ObjectPool> auth) noexcept
    {
        m_bindings.clear();
    }

    void MaterialBindings::setSelfHandle(Authority<ObjectPool> author, MaterialBindingsHandle handle) noexcept
    {
        m_handle = handle;
    }

    MaterialBindingsHandle MaterialBindings::getHandle() const noexcept
    {
        return m_handle;
    }

    uint32_t MaterialBindings::getBindingsCount() const noexcept
    {
        return static_cast<uint32_t>(m_bindings.size());
    }

    std::optional<MaterialBinding> MaterialBindings::getBinding(uint32_t index) const noexcept
    {
        if (index >= static_cast<uint32_t>(m_bindings.size()))
        {
            return std::nullopt;
        }

        return m_bindings[index];
    }

    std::vector<MaterialBinding> const& MaterialBindings::getBindings() const noexcept
    {
        return m_bindings;
    }

    Material* MaterialBindings::getBoundMaterial(uint32_t index) const noexcept
    {
        if ((m_pObjectPool == nullptr) || (index >= static_cast<uint32_t>(m_bindings.size())))
        {
            return nullptr;
        }

        return m_pObjectPool->getMaterial(m_bindings[index].handle);
    }

    bool MaterialBindings::setBinding(uint32_t index, MaterialBinding binding) noexcept
    {
        if (index >= static_cast<uint32_t>(m_bindings.size()))
        {
            return false;
        }

        m_bindings[index] = binding;

        return true;
    }
}