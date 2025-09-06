#pragma once

#include "assets/iasset.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <span>

#include <glm/glm.hpp>

struct CharacterGlyph {
    glm::ivec2 size;
    glm::ivec2 bearing;  // offset from baseline to left/top
    uint32_t advance;    // offset to advance to next
    glm::vec2 uv_0;      // top-left UV coordinates in the font texture
    glm::vec2 uv_1;      // bottom-right UV coordinates in the font texture
};

class FontAsset : public IAsset {
public:
    // Used when loading font from file
    FontAsset(std::string name);

    static std::shared_ptr<FontAsset> create_fallback();

    static std::optional<std::shared_ptr<FontAsset>> load_from_file(const std::string& path, const std::string& name);
    static const char* base_path() {
        return "assets/fonts/";
    }

    bool has_char(uint8_t c) const;
    void compute_vertices(uint8_t c, float* x, float y, float scale, std::span<float, 16> vertices) const;

    const std::string& name() const;
    AssetID texture_id() const;

protected:
    std::ostream& print(std::ostream& os) const override;

private:
    std::string m_name;
    AssetID m_texture;
    std::unordered_map<uint8_t, CharacterGlyph> m_chars;
};
