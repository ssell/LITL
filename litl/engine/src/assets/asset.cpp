#include "litl-engine/assets/asset.hpp"

namespace litl
{

    Asset::Asset()
    {

    }

    Asset::Asset(Asset const& other) noexcept
    {
        locator = other.locator;
        key = other.key;
        hashedKey = other.hashedKey;
        type = other.type;
        status.store(other.status.load(std::memory_order::relaxed), std::memory_order::relaxed);
        error = other.error;
        assetOps = other.assetOps;
    }

    Asset& Asset::operator=(Asset const& other) noexcept
    {
        locator = other.locator;
        key = other.key;
        hashedKey = other.hashedKey;
        type = other.type;
        status.store(other.status.load(std::memory_order::relaxed), std::memory_order::relaxed);
        error = other.error;
        assetOps = other.assetOps;

        return *this;
    }

    Asset::Asset(Asset&& other) noexcept
    {
        if (this != &other)
        {
            locator = other.locator;
            key = other.key;
            hashedKey = other.hashedKey;
            type = other.type;
            status.store(other.status.load(std::memory_order::relaxed), std::memory_order::relaxed);
            error = other.error;
            assetOps = other.assetOps;
        }
    }

    Asset& Asset::operator=(Asset&& other) noexcept
    {
        if (this != &other)
        {
            locator = other.locator;
            key = other.key;
            hashedKey = other.hashedKey;
            type = other.type;
            status.store(other.status.load(std::memory_order::relaxed), std::memory_order::relaxed);
            error = other.error;
            assetOps = other.assetOps;
        }

        return *this;
    }

    Asset::~Asset()
    {

    }

    void Asset::setError(AssetErrorCode err) noexcept
    {
        error = err;
        status.store(AssetStatus::Error, std::memory_order::memory_order_release);
    }

    void Asset::setError(AssetErrorCode err, AssetErrorCode def) noexcept
    {
        error = (err != AssetErrorCode::None ? err : def);
        status.store(AssetStatus::Error, std::memory_order::memory_order_release);
    }
}