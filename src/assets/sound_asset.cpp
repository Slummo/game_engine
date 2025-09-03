#include "assets/sound_asset.h"
#include "core/log.h"

#include <sndfile.h>
#include <AL/al.h>

std::optional<std::shared_ptr<SoundAsset>> SoundAsset::load_from_file(const std::string& path) {
    SF_INFO info;
    SNDFILE* file = sf_open(path.c_str(), SFM_READ, &info);
    if (!file) {
        ERR("[SoundAsset] Failed to load sound " << sf_strerror(NULL));
        return std::nullopt;
    }

    std::vector<int16_t> pcm(info.frames * info.channels);
    sf_readf_short(file, pcm.data(), info.frames);
    sf_close(file);

    auto sound = std::make_shared<SoundAsset>();
    if (!sound->upload(info.channels, pcm.data(), pcm.size() * sizeof(int16_t), info.samplerate)) {
        ERR("[SoundAsset] Failed to upload sound: " << path);
        return std::nullopt;
    }

    sound->m_channels = info.channels;
    sound->m_samplerate = info.samplerate;
    sound->m_full_path = std::string(path);

    return sound;
}

const std::string& SoundAsset::full_path() {
    return m_full_path;
}

std::shared_ptr<SoundAsset> SoundAsset::create_fallback() {
    return std::make_shared<SoundAsset>();
}

SoundAsset::~SoundAsset() {
    alDeleteBuffers(1, &m_buffer_id);
}

uint32_t SoundAsset::buffer_id() const {
    return m_buffer_id;
}

int32_t SoundAsset::channels() const {
    return m_channels;
}

int32_t SoundAsset::samplerate() const {
    return m_samplerate;
}

std::ostream& SoundAsset::print(std::ostream& os) const {
    return os << "SoundAsset(buffer_id: " << m_buffer_id << ", path: " << m_full_path << ")";
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