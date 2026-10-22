#ifndef LITL_PLATFORM_H__
#define LITL_PLATFORM_H__

#include <cstdint>

#include "litl-platform/window/window.hpp"
#include "litl-platform/input/input.hpp"
#include "litl-platform/platformConfiguration.hpp"

namespace litl
{
    /// <summary>
    /// Explicitly not defined in litl-platform, but instead is defined in the implementation (litl-platform-glfw, etc.)
    /// </summary>
    struct PlatformContext;
    class Platform;

    struct PlatformOps
    {
        bool (*initialize)(PlatformContext*);
        bool (*shutdown)(PlatformContext*);
        Window* (*createWindow)(Platform*, PlatformContext*, WindowConfiguration const&);
        Window* (*getWindow)(PlatformContext*);
        Input* (*createInput)(PlatformContext*, InputConfiguration const&);
        Input* (*getInput)(PlatformContext*);
    };

    class Platform final
    {
    public:

        Platform();
        ~Platform();

        Platform(Platform const&) = delete;
        Platform& operator=(Platform const&) = delete;

        [[nodiscard]] bool setup(PlatformOps const* operations, PlatformContext* context) noexcept;
        [[nodiscard]] bool initialize() const noexcept;
        [[nodiscard]] bool shutdown() const noexcept;
        [[nodiscard]] Window* createWindow(WindowConfiguration const& config) noexcept;
        [[nodiscard]] Window* getWindow() const noexcept;
        [[nodiscard]] Input* createInput(InputConfiguration const& config) const noexcept;
        [[nodiscard]] Input* getInput() const noexcept;

    private:

        PlatformOps const* m_pOps{ nullptr };
        PlatformContext* m_pContext{ nullptr };
    };
}

#endif