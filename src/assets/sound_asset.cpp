#include "assets/sound_asset.h"
#include "managers/asset_manager.h"

#include <sndfile.h>
#include <AL/al.h>

SoundAsset::SoundAsset(std::string name) : IAsset(name.empty() ? "unnamed_sound" : std::move(name)) {
}

uint32_t SoundAsset::buffer_id() const {
    return m_buffer_id;
}

SoundAsset::~SoundAsset() {
    if (m_buffer_id) {
        alDeleteBuffers(1, &m_buffer_id);
        m_buffer_id = 0;
    }
}

std::ostream& SoundAsset::print(std::ostream& os) const {
    return os << "SoundAsset(m_buffer_id: " << m_buffer_id << ")";
}

bool SoundAsset::upload(int32_t channels, const void* data, int32_t size, int32_t samplerate) {
    if (channels <= 0 || size <= 0 || samplerate <= 0 || !data) {
        ERR("[SoundAsset] Invalid arguments in upload");
        return false;
    }

    ALenum format = AL_FORMAT_MONO16;
    if (channels == 2) {
        format = AL_FORMAT_STEREO16;
    } else if (channels == 1) {
        format = AL_FORMAT_MONO16;
    } else {
        ERR("[SoundAsset] Unsupported channel count: " << channels);
        return false;
    }

    alGenBuffers(1, &m_buffer_id);
    alBufferData(m_buffer_id, format, data, size, samplerate);

    return true;
}

std::shared_ptr<SoundAsset> AssetCreator<SoundAsset>::create_fallback(AssetManager& /*am*/) {
    return std::make_shared<SoundAsset>("fallback_sound");
}

const char* AssetLoader<SoundAsset>::base_path() {
    return "assets/sounds/";
}

AssetID AssetLoader<SoundAsset>::finish() {
    AssetID id = am.is_loaded(absolute_path);
    if (id != INVALID_ASSET) {
        return id;
    }

    SF_INFO info;
    SNDFILE* file = sf_open(absolute_path.c_str(), SFM_READ, &info);
    if (!file) {
        ERR("[AssetLoader<SoundAsset>] Failed to load sound " << sf_strerror(nullptr));
        return INVALID_ASSET;
    }

    std::vector<int16_t> pcm(info.frames * info.channels);
    sf_readf_short(file, pcm.data(), info.frames);
    sf_close(file);

    auto sound = std::make_shared<SoundAsset>(std::move(name));
    if (!sound->upload(info.channels, pcm.data(), pcm.size() * sizeof(int16_t), info.samplerate)) {
        ERR("[AssetLoader<SoundAsset>] Failed to upload sound: " << absolute_path);
        return INVALID_ASSET;
    }

    sound->channels = info.channels;
    sound->samplerate = info.samplerate;

    id = am.add<SoundAsset>(std::move(sound));
    am.add_loaded(std::move(absolute_path), id);
    return id;
}