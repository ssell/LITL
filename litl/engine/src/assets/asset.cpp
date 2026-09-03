#include "litl-engine/assets/asset.hpp"

namespace litl
{

    Asset::Asset()
    {

    }

    Asset::Asset(Asset const& other) noexcept
    {
        file = other.file;
        key = other.key;
        hashedKey = other.hashedKey;
        type = other.type;
        status.store(other.status.load(std::memory_order_relaxed), std::memory_order_relaxed);
        error = other.error;
        assetOps = other.assetOps;
    }

    Asset& Asset::operator=(Asset const& other) noexcept
    {
        file = other.file;
        key = other.key;
        hashedKey = other.hashedKey;
        type = other.type;
        status.store(other.status.load(std::memory_order_relaxed), std::memory_order_relaxed);
        error = other.error;
        assetOps = other.assetOps;

        return *this;
    }

    Asset::Asset(Asset&& other) noexcept
    {
        if (this != &other)
        {
            file = other.file;
            key = std::move(other.key);
            hashedKey = std::exchange(other.hashedKey, {});
            type = std::exchange(other.type, AssetType::Unknown);
            status.store(other.status.load(std::memory_order_relaxed), std::memory_order_relaxed);
            other.status.store(AssetStatus::Unloaded, std::memory_order_relaxed);
            error = std::exchange(other.error, AssetErrorCode::None);
            assetOps = std::exchange(other.assetOps, nullptr);
        }
    }

    Asset& Asset::operator=(Asset&& other) noexcept
    {
        if (this != &other)
        {
            file = other.file;
            key = std::move(other.key);
            hashedKey = std::exchange(other.hashedKey, {});
            type = std::exchange(other.type, AssetType::Unknown);
            status.store(other.status.load(std::memory_order_relaxed), std::memory_order_relaxed);
            other.status.store(AssetStatus::Unloaded, std::memory_order_relaxed);
            error = std::exchange(other.error, AssetErrorCode::None);
            assetOps = std::exchange(other.assetOps, nullptr);
        }

        return *this;
    }

    Asset::~Asset()
    {

    }

    void Asset::setError(AssetErrorCode err) noexcept
    {
        status.store(AssetStatus::Error, std::memory_order_relaxed);
        error = err;
    }

    void Asset::setError(AssetErrorCode err, AssetErrorCode def) noexcept
    {
        status.store(AssetStatus::Error, std::memory_order_relaxed);
        error = (err != AssetErrorCode::None ? err : def);
    }
}