#include "litl-import/material/intermediate/materialIntermediateData.hpp"

namespace litl::import
{
    void MaterialIntermediateData::setName(std::string_view name) noexcept
    {
        // ... todo ...
    }

    bool MaterialIntermediateData::setShader(LitlMatShaderStage stage, std::string const& resource, std::string const& entry) noexcept
    {
        if (stage == LitlMatShaderStage::Unknown)
        {
            return false;
        }

        if (resource.empty())
        {
            return false;
        }

        if (entry.empty())
        {
            return false;
        }

        m_shaders[static_cast<uint32_t>(stage) - 1] = LitlMatShaderRecord{
            .stage = stage,
            .resource = resource,
            .entry = entry
        };

        return true;
    }

    bool MaterialIntermediateData::addProperty(std::string const& name, LitlMatPropertyType type, LitlMatSupportedRawPropertyTypes const& value) noexcept
    {
        switch (type)
        {
        case LitlMatPropertyType::Bool:
            if (std::holds_alternative<uint8_t>(value)) { m_properties.push_back(LitlMatPropertyRecord{ .name = name, type, std::get<uint8_t>(value) }); return true; }
            else if (std::holds_alternative<bool>(value)) { m_properties.push_back(LitlMatPropertyRecord{ .name = name, type, static_cast<uint8_t>(std::get<bool>(value)) }); return true; }
            break;

        case LitlMatPropertyType::Integer:
            if (std::holds_alternative<int32_t>(value)) { m_properties.push_back(LitlMatPropertyRecord{ .name = name, type, std::get<int32_t>(value) }); return true; }
            break;

        case LitlMatPropertyType::UnsignedInteger:
            if (std::holds_alternative<uint32_t>(value)) { m_properties.push_back(LitlMatPropertyRecord{ .name = name, type, std::get<uint32_t>(value) }); return true; }
            break;

        case LitlMatPropertyType::Float:
            if (std::holds_alternative<float>(value)) { m_properties.push_back(LitlMatPropertyRecord{ .name = name, type, std::get<float>(value) }); return true; }
            break;

        case LitlMatPropertyType::Double:
            if (std::holds_alternative<double>(value)) { m_properties.push_back(LitlMatPropertyRecord{ .name = name, type, std::get<double>(value) }); return true; }
            break;

        case LitlMatPropertyType::Vec2:
            if (!std::holds_alternative<vec2>(value))
            {
                if (auto* vec = std::get_if<std::vector<float>>(&value))
                {
                    if (vec->size() >= 2)
                    {
                        m_properties.push_back(LitlMatPropertyRecord{ .name = name, type, vec2(vec->at(0), vec->at(1)) });
                        return true;
                    }
                }
            }
            else
            {
                m_properties.push_back(LitlMatPropertyRecord{ .name = name, type, std::get<vec2>(value) });
                return true;
            }
            break;

        case LitlMatPropertyType::Vec3:
            if (!std::holds_alternative<vec3>(value))
            {
                if (auto* vec = std::get_if<std::vector<float>>(&value))
                {
                    if (vec->size() >= 3)
                    {
                        m_properties.push_back(LitlMatPropertyRecord{ .name = name, type, vec3(vec->at(0), vec->at(1), vec->at(2)) });
                        return true;
                    }
                }
            }
            else
            {
                m_properties.push_back(LitlMatPropertyRecord{ .name = name, type, std::get<vec3>(value) });
                return true;
            }
            break;

        case LitlMatPropertyType::Vec4:
            if (!std::holds_alternative<vec4>(value))
            {
                if (auto* vec = std::get_if<std::vector<float>>(&value))
                {
                    if (vec->size() >= 4)
                    {
                        m_properties.push_back(LitlMatPropertyRecord{ .name = name, type, vec4(vec->at(0), vec->at(1), vec->at(2), vec->at(3)) });
                        return true;
                    }
                }
            }
            else
            {
                m_properties.push_back(LitlMatPropertyRecord{ .name = name, type, std::get<vec4>(value) });
                return true;
            }
            break;

        case LitlMatPropertyType::Color:
            if (!std::holds_alternative<color>(value))
            {
                if (auto* vec = std::get_if<std::vector<float>>(&value))
                {
                    if (vec->size() >= 3)
                    {
                        m_properties.push_back(LitlMatPropertyRecord{ .name = name, type, color(vec->at(0), vec->at(1), vec->at(2), (vec->size() >= 4 ? vec->at(3) : 1.0f)) });
                        return true;
                    }
                }
            }
            else
            {
                m_properties.push_back(LitlMatPropertyRecord{ .name = name, type, std::get<color>(value) });
                return true;
            }
            break;

        case LitlMatPropertyType::Texture2D:
        case LitlMatPropertyType::Texture3D:
            if (std::holds_alternative<std::string>(value)) { m_properties.push_back(LitlMatPropertyRecord{ .name = name, type, std::get<std::string>(value) }); return true; }
            break;

        case LitlMatPropertyType::Unknown:
            break;
        }

        return false;
    }

    void MaterialIntermediateData::setRasterCullMode(LitlMatCullMode cullMode) noexcept
    {
        m_rasterSettings.cullMode = cullMode;
    }

    void MaterialIntermediateData::setRasterWinding(bool clockwise) noexcept
    {
        m_rasterSettings.clockwise = clockwise;
    }

    void MaterialIntermediateData::setHintFrequentUpdates(bool frequentUpdates) noexcept
    {
        m_hintSettings.frequentUpdates = frequentUpdates;
    }

    std::array<LitlMatShaderRecord, 7> const& MaterialIntermediateData::getShaders() const noexcept
    {
        return m_shaders;
    }

    std::vector<LitlMatPropertyRecord> const& MaterialIntermediateData::getProperties() const noexcept
    {
        return m_properties;
    }

    LitlMatRasterSettings const& MaterialIntermediateData::getRasterSettings() const noexcept
    {
        return m_rasterSettings;
    }

    LitlMatHintSettings const& MaterialIntermediateData::getHintSettings() const noexcept
    {
        return m_hintSettings;
    }
}