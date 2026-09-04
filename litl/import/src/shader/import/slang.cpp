#include <cstdint>
#include <expected>
#include <format>
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <string>
#include <string_view>
#include <vector>

#include "litl-core/logging/logging.hpp"
#include "litl-import/shader/import/slang.hpp"

namespace litl::import
{
    namespace
    {
        struct SpirvModule
        {
            std::vector<std::uint32_t> words;
            std::string warnings;
        };

        static thread_local std::expected<Slang::ComPtr<slang::ISession>, std::string> t_slangSession = nullptr;

        std::expected<Slang::ComPtr<slang::ISession>, std::string> createSlangSession() noexcept
        {
            Slang::ComPtr<slang::IGlobalSession> global;

            if (SLANG_FAILED(slang::createGlobalSession(global.writeRef())))
            {
                return std::unexpected("Failed to create Slang Global session.");
            }

            const slang::TargetDesc targetDesc{
                .format = SLANG_SPIRV,
                .profile = global->findProfile("spirv_1_5"),            // for Vulkan 1.4
                .flags = 0
            };

            const slang::SessionDesc sessionDesc{
                .targets = &targetDesc,
                .targetCount = 1,
                .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR
            };

            Slang::ComPtr<slang::ISession> session;

            if (SLANG_FAILED(global->createSession(sessionDesc, session.writeRef())))
            {
                return std::unexpected("Failed to create Slang session.");
            }

            return session;
        }

        std::expected<SpirvModule, std::string> compileSlang(
            slang::ISession* session,
            std::string_view fileName,                              // "flat"
            std::string_view fileImportPath,                        // "shaders/flat.slang" (for diagnostics)
            std::span<std::byte const> sourceBytes) noexcept
        {
            auto slangDiagnosticsText = [](slang::IBlob* blob) noexcept -> std::string { return (blob ? std::string(static_cast<const char*>(blob->getBufferPointer())) : std::string{}); };
            
            // Transform std::byte span to std::string and gain null termination.
            const std::string slangText(reinterpret_cast<const char*>(sourceBytes.data()), sourceBytes.size());

            /**
             * Note / todo:
             * 
             * loadModule takes a module name, not strictly a path. 
             * Passing "shaders/flat.slang" works, but the clean form is to add the shader root to searchPaths and pass "flat". 
             * That also makes `import pbr;` inside other shaders resolve consistently.
             */

            // Load the slang text. On failure imodule will be null.
            Slang::ComPtr<slang::IBlob> diagnostics;
            slang::IModule* imodule = session->loadModuleFromSourceString(
                std::string(fileName).c_str(),
                std::string(fileImportPath).c_str(),
                slangText.c_str(),
                diagnostics.writeRef());

            if (imodule == nullptr)
            {
                return std::unexpected(std::format("Slang module '{}' error during loadModuleFromSourceString: {}", fileName, slangDiagnosticsText(diagnostics)));
            }

            std::string warnings = slangDiagnosticsText(diagnostics);
            
            if (!warnings.empty())
            {
                warnings = std::format("Slang module '{}' warnings during loadModuleFromSourceString: {}", fileName, warnings);
            }

            // Collect the various shader entry points. For example: [shader("vertexMain")]
            std::vector<slang::IComponentType*> components{ imodule };
            std::vector<Slang::ComPtr<slang::IEntryPoint>> entryPoints;

            const auto entryPointCount = imodule->getDefinedEntryPointCount();
            entryPoints.reserve(entryPointCount);

            for (SlangInt i = 0; i < entryPointCount; ++i)
            {
                auto& entryPoint = entryPoints.emplace_back();

                if (SLANG_FAILED(imodule->getDefinedEntryPoint(i, entryPoint.writeRef())))
                {
                    return std::unexpected(std::format("Slang module '{}' getDefinedEntryPoint({}) failed.", fileName, static_cast<uint32_t>(i)));
                }

                components.push_back(entryPoint.get());
            }

            if (components.size() == 1)
            {
                return std::unexpected(std::format("Slang module '{}' has no entry points.", fileName));
            }

            // Compose module and entry points into one program
            Slang::ComPtr<slang::IComponentType> composed;
            diagnostics.setNull();

            if (SLANG_FAILED(session->createCompositeComponentType(
                components.data(),
                static_cast<SlangInt>(components.size()),
                composed.writeRef(),
                diagnostics.writeRef())))
            {
                return std::unexpected(std::format("Slang module '{}' error during createCompositeComponentType: {}", fileName, slangDiagnosticsText(diagnostics)));
            }

            // Link: resolve specializations, generics, cross-module references.
            Slang::ComPtr<slang::IComponentType> linked;
            diagnostics.setNull();

            if (SLANG_FAILED(composed->link(linked.writeRef(), diagnostics.writeRef())))
            {
                return std::unexpected(std::format("Slang module '{}' error during link: {}", fileName, slangDiagnosticsText(diagnostics)));
            }

            // Emit: target index 0 = the single SPIR-V target in the SessionDesc.
            Slang::ComPtr<slang::IBlob> code;
            diagnostics.setNull();

            if (SLANG_FAILED(linked->getTargetCode(0, code.writeRef(), diagnostics.writeRef())))
            {
                return std::unexpected(std::format("Slang module '{}' error during getTargetCode: {}", fileName, slangDiagnosticsText(diagnostics)));
            }

            // Copy out. The blob will die with the ComPtr.
            const auto bytes = code->getBufferSize();
            const auto* words = static_cast<const uint32_t*>(code->getBufferPointer());

            if ((bytes % 4) != 0)
            {
                return std::unexpected(std::format("Slang module '{}' error: SPIR-V blob is not word-aligned.", fileName));
            }

            return SpirvModule{
                .words = std::vector<uint32_t>(words, words + (bytes / 4)),
                .warnings = std::move(warnings)
            };
        }
    }

    SlangImporter::SlangImporter()
    {

    }

    SlangImporter::~SlangImporter()
    {

    }

    Result SlangImporter::import(File const& file, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        if (t_slangSession == nullptr)
        {
            t_slangSession = createSlangSession();

            if (!t_slangSession)
            {
                return Result::Error(ErrorType::ImporterFailed, std::format("Failed to retrieve thread-local Slang session with error: {}", t_slangSession.error()));
            }
        }

        auto spirvModule = compileSlang(t_slangSession.value(), file.name(), "", sourceBytes);

        if (!spirvModule)
        {
            return Result::Error(ErrorType::ImporterFailed, std::format("Failed to compile .slang to SPIR-V with error: {}", spirvModule.error()));
        }

        if (!spirvModule->warnings.empty())
        {
            logWarning("Slang SPIR-V compilation warning for '", file.localPath(), "': ", spirvModule->warnings);
        }

        if (!importedData.setType(ImportedDataType::Shader))
        {
            return Result::Error(ErrorType::ImporterFailed, "Failed to create mesh import data.");
        }

        auto* shader = importedData.getDataPtr<ShaderImportResult>();
        shader->intermediateShader = std::make_unique<ShaderIntermediateData>();
        shader->intermediateShader->setSpirvWords(spirvModule->words);


        return Result::Success();
    }
}