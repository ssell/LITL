#include <mutex>

#include "litl-engine/cache/pipelineLayoutCache.hpp"
#include "litl-core/containers/flatHashMap.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/refPtr.hpp"
#include "litl-renderer/pipeline/pipelineLayout.hpp"

namespace LITL::Engine
{
    struct PipelineLayoutCache::Impl
    {
        Renderer::Renderer const* pRenderer;
        Core::FlatHashMap<uint64_t, std::unique_ptr<Renderer::PipelineLayout>> cache;
        std::mutex cacheMutex;
    };

    PipelineLayoutCache::PipelineLayoutCache(LITL::Renderer::Renderer const* pRenderer)
    {
        m_impl->pRenderer = pRenderer;
    }

    PipelineLayoutCache::~PipelineLayoutCache()
    {

    }

    Renderer::PipelineLayout* PipelineLayoutCache::getPipelineLayout(Renderer::PipelineLayoutDescriptor const& descriptor)
    {
        std::lock_guard<std::mutex> lock(m_impl->cacheMutex);

        auto cached = m_impl->cache.find(descriptor.hash());

        if (cached != std::nullopt)
        {
            return cached->get().get();
        }
        else
        {
            auto newObj = m_impl->pRenderer->createPipelineLayout();
            auto raw = newObj.get();
            
            if (m_impl->cache.insert(descriptor.hash(), std::move(newObj)))
            {
                return raw;
            }
            else
            {
                // Failed to cache for some reason?
                logError("Failed to cache PipelineLayout with hash ", descriptor.hash());
                
            }
        }

        return nullptr;
    }
}