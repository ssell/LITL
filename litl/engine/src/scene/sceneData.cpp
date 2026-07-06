#include "litl-core/assert.hpp"
#include "litl-engine/scene/sceneData.hpp"

namespace litl
{
    SceneData::SceneData()
    {
        m_cameraDatums.reserve(32u);
    }

    void SceneData::trackCameraForFrame(Entity entity) noexcept
    {
        for (auto tracked : m_cameraDatums)
        {
            if (tracked.entity == entity)
            {
                // Already known
                return;
            }
        }

        m_cameraDatums.emplace_back(entity);
    }

    CameraData& SceneData::getCameraData(Entity entity) noexcept
    {
        for (auto tracked : m_cameraDatums)
        {
            if (tracked.entity == entity)
            {
                // Already known
                return tracked.data;
            }
        }

        LITL_ASSERT_MSG(false, "Requested to retrieve Camera Data for entity that does not have a tracked Camera component.", {});
    }

    CameraData const& SceneData::getCameraData(Entity entity) const noexcept
    {
        for (auto tracked : m_cameraDatums)
        {
            if (tracked.entity == entity)
            {
                // Already known
                return tracked.data;
            }
        }

        LITL_ASSERT_MSG(false, "Requested to retrieve Camera Data for entity that does not have a tracked Camera component.", {});
    }
}