#include "tests.hpp"
#include "litl-platform/inputSource.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("inputSource factories", "[engine::input]")
    {
        const auto keySource = InputSource::fromKey(Key::Space);
        REQUIRE(keySource.type == InputSourceType::Key);
        REQUIRE(static_cast<Key>(keySource.code) == Key::Space);

        const auto mouseButtonSource = InputSource::fromMouseButton(MouseButton::Left);
        REQUIRE(mouseButtonSource.type == InputSourceType::MouseButton);
        REQUIRE(static_cast<MouseButton>(mouseButtonSource.code) == MouseButton::Left);

        const auto mouseDeltaSource = InputSource::fromMouseDelta();
        REQUIRE(mouseDeltaSource.type == InputSourceType::MouseDelta);
        REQUIRE(mouseDeltaSource.code == Constants::uint32_null_index);

        const auto mouseScrollSource = InputSource::fromMouseScroll();
        REQUIRE(mouseScrollSource.type == InputSourceType::MouseScroll);
        REQUIRE(mouseScrollSource.code == Constants::uint32_null_index);

        const auto gamepadButtonSource = InputSource::fromGamepadButton(GamepadButton::A);
        REQUIRE(gamepadButtonSource.type == InputSourceType::GamepadButton);
        REQUIRE(static_cast<GamepadButton>(gamepadButtonSource.code) == GamepadButton::A);

        const auto gamepadAxisSource = InputSource::fromGamepadAxis(GamepadAxis::LeftTrigger);
        REQUIRE(gamepadAxisSource.type == InputSourceType::GamepadAxis);
        REQUIRE(static_cast<GamepadAxis>(gamepadAxisSource.code) == GamepadAxis::LeftTrigger);
    } LITL_END_TEST_CASE
}