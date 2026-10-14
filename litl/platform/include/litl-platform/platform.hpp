#ifndef LITL_PLATFORM_H__
#define LITL_PLATFORM_H__

#include <cstdint>

#include "litl-core/authority.hpp"
#include "litl-platform/window.hpp"

namespace litl
{
    /// <summary>
    /// Explicitly not defined in litl-platform, but instead is defined in the implementation (litl-platform-glfw, etc.)
    /// </summary>
    struct PlatformContext;

    struct PlatformOps
    {
        bool (*initialize)(PlatformContext*);
        bool (*shutdown)(PlatformContext*);
        Window* (*createWindow)(PlatformContext*);
        Window* (*getWindow)(PlatformContext*);
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
        [[nodiscard]] Window* createWindow() const noexcept;
        [[nodiscard]] Window* getWindow() const noexcept;



    private:

        PlatformOps const* m_pOps{ nullptr };
        PlatformContext* m_pContext{ nullptr };
    };
}

#endif