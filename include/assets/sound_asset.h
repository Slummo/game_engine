#pragma once

#include "assets/interfaces.h"

#include <memory>
#include <string>
#include <vector>
#include <optional>

class SoundAsset : public IAsset {
public:
    SoundAsset(std::string name);

    uint32_t buffer_id() const;

    ~SoundAsset();

    int32_t channels;
    int32_t samplerate;

    bool upload(int32_t channels, const void* data, int32_t size, int32_t samplerate);

protected:
    std::ostream& print(std::ostream& os) const override;

private:
    uint32_t m_buffer_id;
};

template <>
class AssetCreator<SoundAsset> : public AssetCreatorNoDep<SoundAsset> {
public:
    using Base = AssetCreatorNoDep<SoundAsset>;
    using Base::Base;

    static std::shared_ptr<SoundAsset> create_fallback(AssetManager& am);
};

template <>
class AssetLoader<SoundAsset> : public AssetLoaderNoDep<SoundAsset> {
public:
    using Base = AssetLoaderNoDep<SoundAsset>;
    using Base::Base;

    static const char* base_path();
    AssetID finish() override;
};
