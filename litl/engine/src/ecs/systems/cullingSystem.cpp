#include <atomic>
#include <vector>

#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/entity/entityCommands.hpp"
#include "litl-engine/ecs/systems/cullingSystem.hpp"
#include "litl-engine/scene/sceneView.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    std::array<CullingBucket, Constants::max_thread_count> CullingSystem::s_cullingBuckets{};
    CullingBucket CullingSystem::s_combinedBucket{};
    bool CullingSystem::s_combined{ false };

    void CullingSystem::setup(ServiceProvider& services)
    {
        m_pObjectPool = services.get<ObjectPool>();
        m_pSceneView = services.get<SceneView>();
    }

    void CullingSystem::prepare()
    {
        // Perform frustum culling for each camera and repopulate the visible entities. Note that this is only a preprocess step - hence why this is a system. 
        // The frustum query gives all entities in the frustum with a transform. But all visible renderable entities is only a subset of that result.

        reset();

        auto cameras = m_pSceneView->getCameras();

        for (uint32_t i = 0u; (i < cameras.size()) && (i < m_cameraVisibleEntities.size()); ++i)
        {
            auto* camera = cameras[i];
            auto& visibleEntities = m_cameraVisibleEntities[i];

            setCameraAtIndex(i, camera);

            m_pSceneView->query(camera->getFrustum(), visibleEntities.entityQueryResults, false);

            for (uint32_t i = 0u; i < static_cast<uint32_t>(visibleEntities.entityQueryResults.size()); ++i)
            {
                visibleEntities.entities[visibleEntities.entityQueryResults[i].entity] = i;
            }
        }
    }

    void CullingSystem::reset() noexcept
    {
        // Clear, but dont destroy, the camera entity containers. Eventually the cameras will reach a high-water mark of entities and the reallocations will stop.
        
        for (auto& cameraEntities : m_cameraVisibleEntities)
        {
            cameraEntities.camera = nullptr;
            cameraEntities.entities.clear();
            cameraEntities.entityQueryResults.clear();
        }

        s_combinedBucket.reset();

        for (auto& cullingBucket : s_cullingBuckets)
        {
            cullingBucket.reset();
        }

        s_combined = false;
    }

    void CullingBucket::reset() noexcept
    {
        for (auto& cameraRenderableEntities : cameraRenderableEntities)
        {
            cameraRenderableEntities.camera = nullptr;
            cameraRenderableEntities.opaque.clear();
            cameraRenderableEntities.transparent.clear();
        }
    }

    uint32_t CullingBucket::activeCameraCount() const noexcept
    {
        uint32_t count = 0u;

        for (auto& cameraRenderableEntities : cameraRenderableEntities)
        {
            if (cameraRenderableEntities.camera == nullptr)
            {
                break;
            }

            count++;
        }

        return count;
    }

    void CullingSystem::setCameraAtIndex(uint32_t i, Camera* camera) noexcept
    {
        m_cameraVisibleEntities[i].camera = camera;
        s_combinedBucket.cameraRenderableEntities[i].camera = camera;

        for (auto& cullingBucket : s_cullingBuckets)
        {
            cullingBucket.cameraRenderableEntities[i].camera = camera;
        }
    }

    void CullingSystem::update(SystemData const& data, Entity entity, Transform const& transform, MeshRef const& meshRef, MaterialRef const* materialRef, VariableMaterialsRef const* variableMaterialsRef)
    {
        if (!meshRef.handle.isValid())
        {
            // Mesh is not valid
            return;
        }

        if (((materialRef == nullptr) || !materialRef->handle.isValid()) && ((variableMaterialsRef == nullptr) || !variableMaterialsRef->handle.isValid()))
        {
            // Neither material option is valid
            return;
        }

        for (uint32_t cameraIndex = 0u; cameraIndex < m_cameraVisibleEntities.size(); ++cameraIndex)
        {
            const auto& cameraVisibleEntities = m_cameraVisibleEntities[cameraIndex];

            if (cameraVisibleEntities.camera == nullptr)
            {
                // We have checked all cameras. Break out.
                break;
            }

            const auto findEntity = cameraVisibleEntities.entities.find(entity);

            if (findEntity != cameraVisibleEntities.entities.end())
            {
                const auto& entityPartition = cameraVisibleEntities.entityQueryResults[findEntity->second];

                // We are visible to this camera, add to our thread-specific culling bucket.
                // Use of a MaterialRef indicates that the entire mesh is drawn with the same material.
                if (materialRef != nullptr)
                {
                    const Material* material = m_pObjectPool->getMaterial(materialRef->handle);

                    if (material != nullptr)
                    {
                        const RenderableEntity renderableEntity{
                            .entity = entity,
                            .meshRef = meshRef,
                            .materialRef = *materialRef,
                            .materialBlendMode = material->getBlendMode(),
                            .position = entityPartition.worldPosition,
                            .distanceSquared = entityPartition.distanceSquared,
                            .firstIndex = 0u,
                            .indexCount = Constants::uint32_null_index
                        };

                        if (renderableEntity.materialBlendMode == MaterialBlendMode::Transparent)
                        {
                            s_cullingBuckets[data.threadIndex].cameraRenderableEntities[cameraIndex].transparent.push_back(renderableEntity);
                        }
                        else
                        {
                            s_cullingBuckets[data.threadIndex].cameraRenderableEntities[cameraIndex].opaque.push_back(renderableEntity);
                        }
                    }
                }
                // Use of a VariableMaterialsRef indicates that only the submeshes with a matching material index are rendered.
                else if (variableMaterialsRef != nullptr)
                {
                    // For variable materials we need to fetch both the mesh and the bindings.
                    auto* mesh = m_pObjectPool->getMesh(meshRef.handle);
                    auto* materialBindings = m_pObjectPool->getMaterialBindings(variableMaterialsRef->handle);

                    if ((mesh != nullptr) && (materialBindings != nullptr))
                    {
                        const auto& submeshes = mesh->getGeoMesh().getSubmeshes();
                        const auto& bindings = materialBindings->getBindings();

                        for (auto i = 0; i < submeshes.size(); ++i)
                        {
                            if (submeshes[i].materialSlot >= bindings.size())
                            {
                                continue;
                            }

                            const auto& binding = bindings[submeshes[i].materialSlot];
                            const Material* material = m_pObjectPool->getMaterial(binding.handle);

                            if (material == nullptr || !binding.slot.isValid())
                            {
                                continue;
                            }

                            const RenderableEntity renderableEntity{
                                .entity = entity,
                                .meshRef = meshRef,
                                .materialRef = MaterialRef {.handle = binding.handle, .slot = binding.slot},
                                .materialBlendMode = material->getBlendMode(),
                                .position = entityPartition.worldPosition,
                                .distanceSquared = entityPartition.distanceSquared,
                                .firstIndex = submeshes[i].firstIndex,
                                .indexCount = submeshes[i].indexCount
                            };

                            if (renderableEntity.materialBlendMode == MaterialBlendMode::Transparent)
                            {
                                s_cullingBuckets[data.threadIndex].cameraRenderableEntities[cameraIndex].transparent.push_back(renderableEntity);
                            }
                            else
                            {
                                s_cullingBuckets[data.threadIndex].cameraRenderableEntities[cameraIndex].opaque.push_back(renderableEntity);
                            }
                        }
                    }
                }
            }
        }
    }

    CullingBucket& CullingSystem::getCombinedCullingBucket() noexcept
    {
        if (!s_combined)
        {
            // For each camera,
            for (uint32_t cameraIndex = 0u; cameraIndex < s_combinedBucket.cameraRenderableEntities.size(); ++cameraIndex)
            {
                auto& cameraRenderableEntities = s_combinedBucket.cameraRenderableEntities[cameraIndex];

                if (cameraRenderableEntities.camera == nullptr)
                {
                    // We have checked all cameras. Break out.
                    break;
                }

                uint32_t totalOpaqueEntities = 0u;
                uint32_t totalTransparentEntities = 0u;

                for (auto& threadCullingBucket : s_cullingBuckets)
                {
                    totalOpaqueEntities += threadCullingBucket.cameraRenderableEntities[cameraIndex].opaque.size();
                    totalTransparentEntities += threadCullingBucket.cameraRenderableEntities[cameraIndex].transparent.size();
                }

                cameraRenderableEntities.opaque.reserve(totalOpaqueEntities);
                cameraRenderableEntities.transparent.reserve(totalTransparentEntities);

                // For each thread,
                for (auto& threadCullingBucket : s_cullingBuckets)
                {
                    // Add the visible renderable entities found for this camera for this thread
                    auto& opaqueEntities = threadCullingBucket.cameraRenderableEntities[cameraIndex].opaque;
                    cameraRenderableEntities.opaque.insert(cameraRenderableEntities.opaque.end(), opaqueEntities.begin(), opaqueEntities.end());

                    auto& transparentEntities = threadCullingBucket.cameraRenderableEntities[cameraIndex].transparent;
                    cameraRenderableEntities.transparent.insert(cameraRenderableEntities.transparent.end(), transparentEntities.begin(), transparentEntities.end());
                }
            }

            s_combined = true;
        }

        return s_combinedBucket;
    }
}