#pragma once

#include "assets/iasset.h"

#include <memory>
#include <string>
#include <cstdint>
#include <vector>
#include <optional>

class Sound : public IAsset {
public:
    Sound() = default;

    static std::shared_ptr<Sound> create_fallback();

    // With sndfile
    static std::optional<std::shared_ptr<Sound>> load_from_file(const std::string& path);
    static const char* base_path() {
        return "assets/sounds/";
    }
    const std::string& full_path();

    ~Sound();

    uint32_t buffer_id() const;
    int32_t channels() const;
    int32_t samplerate() const;

protected:
    std::ostream& print(std::ostream& os) const override;

private:
    uint32_t m_buffer_id;
    int32_t m_channels;
    int32_t m_samplerate;
    std::string m_full_path;

    bool upload(int32_t channels, const void* data, int32_t size, int32_t samplerate);
};
