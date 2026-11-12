#include "litl-engine/startup.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "freeFlyController.hpp"
#include "spinSystem.hpp"

namespace litl::samples
{
    [[nodiscard]] Configuration buildConfig() noexcept
    {
        return Configuration{
            .platform = PlatformConfiguration {
                .window = WindowConfiguration {
                    .title = "LITL - Sponza Sample"
                },
                .input = InputConfiguration {
                    .actionMaps = {
                        // UI action map with a normal cursor that listens for a click. On click transitions to the gameplay map.
                        ActionMapDescriptor{
                            .name = "ui",
                            .propagateActions = false,
                            .cursorMode = CursorMode::Normal,
                            .bindings = {
                                { "click", { InputSource::fromMouseButton(MouseButton::Left) } }
                            }
                        },
                        // Gameplay map for capturing events relevant to the free-fly camera. On "escape" event, returns back to the UI map to free the cursor.
                        ActionMapDescriptor{
                            .name = "gameplay",
                            .propagateActions = false,
                            .cursorMode = CursorMode::Disabled,
                            .bindings = {
                                { "forward", { InputSource::fromKey(Key::W), InputSource::fromKey(Key::Up) } },
                                { "backward", { InputSource::fromKey(Key::S), InputSource::fromKey(Key::Down) } },
                                { "left", { InputSource::fromKey(Key::A), InputSource::fromKey(Key::Left) } },
                                { "right", { InputSource::fromKey(Key::D), InputSource::fromKey(Key::Right) } },
                                { "up", { InputSource::fromKey(Key::E), InputSource::fromKey(Key::Space) }},
                                { "down", { InputSource::fromKey(Key::Q) } },
                                { "look", { InputSource::fromMouseDelta() } },
                                { "sprint", {InputSource::fromKey(Key::LeftShift), InputSource::fromKey(Key::RightShift) } },
                                { "escape", { InputSource::fromKey(Key::Escape) } }
                            }
                        }
                    },
                    .defaultActionMap = "ui"
                }
            }
        };
    }

    void configureSystems(SystemCollection& systems)
    {
        systems.addSystem<FreeFlyControllerSystem>(SystemGroup::Update);
        systems.addSystem<SpinSystem>(SystemGroup::Update);
    }

    void configureCallbacks(BootstrapCallbacks& callbacks)
    {
        callbacks.inputCallbacks->registerActionMapCallback("ui"_sid, "click"_sid, [](ServiceProvider& services, ActionMapState actionState)
            {
                if (actionState.released)
                {
                    auto input = services.get<Input>();
                    input->pushMap(input->getMap("gameplay"_sid));
                }

                return InputCallbackResult::Continue;
            });

        callbacks.inputCallbacks->registerActionMapCallback("gameplay"_sid, "escape"_sid, [](ServiceProvider& services, ActionMapState actionState)
            {
                if (actionState.released)
                {
                    auto input = services.get<Input>();
                    input->popMap(input->getMap("gameplay"_sid));
                }

                return InputCallbackResult::Continue;
            });
    }

    void cameraBootstrap(EntityCommands& commands, std::shared_ptr<ObjectPool> objectPool, std::shared_ptr<SceneView> sceneView, std::shared_ptr<AssetManager> assets, bool addAltCamera) noexcept
    {
        const auto* mainCamera = createMainCamera(color{ 0.015f, 0.015f, 0.025f }, vec3(-5.0f, 1.5f, 0.25f), vec3(5.0f, 1.5f, 0.25f), vec3::up(), *objectPool, *sceneView);
        commands.addComponent<FreeFlyController>(mainCamera->getEntity());
        commands.addComponent<PendingModelInstance>(mainCamera->getEntity(), createModelInstance("litl/models/sphere", "litl/materials/lit", *assets));
        commands.addComponent<LocalBounds>(mainCamera->getEntity());

        if (addAltCamera)
        {
            // Secondary camera to show views are working that is presented in the upper-right corner
            const auto altCameraHandle = objectPool->createCamera(CameraDescriptor{
                .objectInfo = {.name = "AltCamera"},
                .type = CameraType::DepthOnly,
                .viewport = ViewportDescriptor{.offset{ 0.75f, 0.0f }, .extents{ 0.25f, 0.25f }},
                .clearColor = color{ 0.015f, 0.25f, 0.025f },
                .zFar = 30.0f,
                .priority = static_cast<uint32_t>(CameraPriority::High)         // High priority so it is drawn _after_ the main camera and so is drawn ontop of it.
                });

            auto* altCamera = objectPool->getCamera(altCameraHandle);

            if (altCamera != nullptr)
            {
                altCamera->setWorldPosition(vec3(0.0f, 1.5f, 0.0f));
                altCamera->lookAt(vec3(5.0f, 1.5f, 0.0f), vec3::up());
            }
        }
    }

    void lightingBootstrap(EntityCommands& commands, std::shared_ptr<LightManager> lights) noexcept
    {
        auto dirLightEntity = commands.createEntity();
        commands.addComponent<Transform>(dirLightEntity, Transform::create(vec3::zero(), quat::fromEuler(60.0f, 100.0f, 0.0f)));
        commands.addComponent<DirectionalLight>(dirLightEntity, DirectionalLight{ .color = color{ 0.95f, 0.95f, 1.0f }, .intensity = 1.0f }); 
        //commands.addComponent<Spin>(dirLightEntity, Spin{ .rate = 50.0f });

        lights->setAmbientColor(color(0.125f, 0.1f, 0.125f, 1.0f));
    }

    void sponzaBootstrap(EntityCommands& commands, std::shared_ptr<AssetManager> assets) noexcept
    {
        auto sponzaEntity = commands.createEntity();
        commands.addComponent<Transform>(sponzaEntity, Transform::create(vec3::zero(), quat::identity()));
        commands.addComponent<PendingModelInstance>(sponzaEntity, createModelInstance("models/ignore/sponza", "litl/materials/lit", *assets));
        commands.addComponent<LocalBounds>(sponzaEntity);
    }

    void transparentCubeBootstrap(EntityCommands& commands, std::shared_ptr<AssetManager> assets) noexcept
    {
        auto cubeEntity = commands.createEntity();
        commands.addComponent<Transform>(cubeEntity, Transform::create(vec3(0.0f, 1.0f, 0.25f), quat::identity()));
        commands.addComponent<PendingModelInstance>(cubeEntity, createModelInstance("litl/models/cube", "litl/materials/litTransparent", *assets));
        commands.addComponent<LocalBounds>(cubeEntity);
        commands.addComponent<Spin>(cubeEntity, Spin{ .rate = 50.0f });
    }

    void pbrSphereGridBootstrap(EntityCommands& commands, std::shared_ptr<AssetManager> assets, std::shared_ptr<RenderManager> renderManager) noexcept
    {
        const float spacing = 2.0f;

        ModelAsset* modelAsset = assets->getModel("litl/models/sphere");
        MaterialAsset* materialAsset = assets->getMaterial("litl/materials/lit");

        if ((modelAsset == nullptr) || (materialAsset == nullptr) || (materialAsset->material == nullptr))
        {
            return;
        }

        PendingModelInstance pendingModel{
            .modelHandle = modelAsset->selfHandle.modelHandle,
            .fallbackMaterialHandle = materialAsset->materialHandle
        };

        for (float roughness = 0.0f; roughness <= 1.0f; roughness += 0.1f)
        {
            for (float metallic = 0.0f; metallic <= 1.0f; metallic += 0.1f)
            {
                const float x = (roughness * 10.0f) * spacing;
                const float z = (metallic * 10.0f) * spacing;

                const auto materialSlot = materialAsset->material->allocateSlot();
                materialAsset->material->setFloat("roughnessFactor"_sid, roughness, materialSlot);
                materialAsset->material->setFloat("metallicFactor"_sid, metallic, materialSlot);
                pendingModel.fallbackMaterialSlot = materialSlot;

                auto pbrSphereEntity = commands.createEntity();
                commands.addComponent<Transform>(pbrSphereEntity, Transform::create(vec3(x, 0.0f, z)));
                commands.addComponent<PendingModelInstance>(pbrSphereEntity, pendingModel);
                commands.addComponent<LocalBounds>(pbrSphereEntity);
            }
        }
    }

    void bootstrap(ServiceProvider& services, EntityCommands& commands)
    {
        auto objectPool = services.get<ObjectPool>();
        auto sceneView = services.get<SceneView>();
        auto assets = services.get<AssetManager>();
        auto lights = services.get<LightManager>();

        cameraBootstrap(commands, objectPool, sceneView, assets, true);
        lightingBootstrap(commands, lights);
        //sponzaBootstrap(commands, assets);
        //transparentCubeBootstrap(commands, assets);
        pbrSphereGridBootstrap(commands, assets, services.get<RenderManager>());
    }
}

int main()
{
    litl::Engine engine{};

    engine.setup(litl::samples::buildConfig(), nullptr, litl::samples::configureSystems, litl::samples::configureCallbacks, litl::samples::bootstrap);
    engine.start();
    engine.shutdown();

    return 0;
}