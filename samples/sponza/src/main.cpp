#include "litl-engine/startup.hpp"
#include "freeFlyController.hpp"
#include "spinSystem.hpp"
#include "litl-platform/window/window.hpp"

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

    void bootstrap(ServiceProvider& services, EntityCommands& commands)
    {
        auto objectPool = services.get<ObjectPool>();
        auto sceneView = services.get<SceneView>();
        auto assets = services.get<AssetManager>();

        const auto* mainCamera = createMainCamera(color{ 0.015f, 0.015f, 0.025f }, vec3(-5.0f, 1.5f, 0.0f), vec3(5.0f, 1.5f, 0.0f), vec3::up(), *objectPool, *sceneView);
        commands.addComponent<FreeFlyController>(mainCamera->getEntity());
        commands.addComponent<PendingModelInstance>(mainCamera->getEntity(), createModelInstance("litl/models/sphere", "litl/materials/lit", *assets));
        commands.addComponent<LocalBounds>(mainCamera->getEntity());

        auto sponzaEntity = commands.createEntity();
        commands.addComponent<Transform>(sponzaEntity, Transform::create(vec3::zero(), quat::identity()));
        commands.addComponent<PendingModelInstance>(sponzaEntity, createModelInstance("models/ignore/sponza", "litl/materials/lit", *assets));
        commands.addComponent<LocalBounds>(sponzaEntity);

        // secondary camera to show views are working
        const auto altCameraRtHandle = objectPool->createRenderTexture(RenderTextureDescriptor{
            .objectInfo = {.name = "AltCamera_RT" },
            .sizePolicy = RenderTextureSizePolicy::SwapchainRelative,
            .colorComponent = true,
            .depthComponent = true
        });

        const auto altCameraHandle = objectPool->createCamera(CameraDescriptor{
            .objectInfo = {.name = "AltCamera"},
            .viewport = VewportDescriptor{.offset{ 0.75f, 0.0f }, .extents{ 0.25f, 0.25f }},
            .clearColor = color{ 0.015f, 0.25f, 0.025f },
            .renderTexture = altCameraRtHandle
        });

        auto* altCamera = objectPool->getCamera(altCameraHandle);

        if (altCamera != nullptr)
        {
            altCamera->setWorldPosition(vec3(-5.0f, 1.5f, 0.0f));
            altCamera->lookAt(vec3(5.0f, 1.5f, 0.0f), vec3::up());
        }

        // Small spinning cube to show something is happening while the Sponza loads in. Replace with GUI text in the future, or remove entirely when asset loading performance is improved.
        //auto cubeEntity = commands.createEntity();
        //commands.addComponent<Transform>(cubeEntity, Transform::create(vec3(5.0f, -2.0f, 0.0f), quat::identity()));
        //commands.addComponent<PendingModelInstance>(cubeEntity, createModelInstance("litl/models/cube", "litl/materials/lit", *assets));
        //commands.addComponent<LocalBounds>(cubeEntity);
        //commands.addComponent<Spin>(cubeEntity);
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