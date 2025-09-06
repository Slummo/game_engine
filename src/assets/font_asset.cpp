#include "assets/font_asset.h"
#include "core/log.h"
#include "managers/asset_manager.h"

#include <cstring>
#include <limits>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

FontAsset::FontAsset(std::string name) : m_name(name.empty() ? "unnamed_font" : std::move(name)) {
}

std::shared_ptr<FontAsset> FontAsset::create_fallback() {
    return std::make_shared<FontAsset>("fallback_font");
}

std::optional<std::shared_ptr<FontAsset>> FontAsset::load_from_file(const std::string& path, const std::string& name) {
    auto font = std::make_shared<FontAsset>(std::string(name));
    AssetManager& am = AssetManager::instance();

    const FT_Library& ft = am.ft_lib();
    FT_Face face;
    if (FT_New_Face(ft, path.c_str(), 0, &face)) {
        ERR("[TextureAsset] Failed to load font: " << path << ")");
        return std::nullopt;
    }
    FT_Set_Pixel_Sizes(face, 0, 48);

    int32_t w = 1024;
    int32_t h = 1024;
    int32_t c = 1;
    std::vector<uint8_t> pixels(w * h, 0);

    int32_t pen_x = 0;
    int32_t pen_y = 0;
    int32_t row_height = 0;

    for (uint8_t c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            continue;
        }

        FT_Bitmap& bitmap = face->glyph->bitmap;

        // Check if we need to wrap to next row
        if (pen_x + bitmap.width >= static_cast<uint32_t>(w)) {
            pen_x = 0;
            pen_y += row_height;
            row_height = 0;
        }

        // Copy glyph bitmap
        for (int32_t y = 0; y < static_cast<int32_t>(bitmap.rows); y++) {
            for (int32_t x = 0; x < static_cast<int32_t>(bitmap.width); x++) {
                int32_t dst_x = pen_x + x;
                int32_t dst_y = pen_y + y;
                pixels[dst_y * w + dst_x] = bitmap.buffer[y * bitmap.pitch + x];
            }
        }

        CharacterGlyph g;
        g.size = glm::ivec2(bitmap.width, bitmap.rows);
        g.bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
        g.advance = face->glyph->advance.x >> 6;  // shift to get the value in pixels
        g.uv_0 = glm::vec2(pen_x / (float)w, (pen_y + bitmap.rows) / (float)h);
        g.uv_1 = glm::vec2((pen_x + bitmap.width) / (float)w, pen_y / (float)h);
        font->m_chars[c] = std::move(g);

        pen_x += bitmap.width + 1;
        row_height = std::max(row_height, static_cast<int32_t>(bitmap.rows));
    }

    FT_Done_Face(face);
    auto tex_opt = TextureAsset::load_from_buffer(w, h, c, pixels);
    if (!tex_opt) {
        ERR("[FontAsset] Failed to load font: " << path);
        return std::nullopt;
    }

    font->m_texture = am.add_asset<TextureAsset>(*tex_opt);

    return font;
}

bool FontAsset::has_char(uint8_t c) const {
    auto it = m_chars.find(c);
    return it != m_chars.end();
}

void FontAsset::compute_vertices(uint8_t c, float* x, float y, float scale, std::span<float, 16> vertices) const {
    auto it = m_chars.find(c);
    if (it == m_chars.end()) {
        return;
    }

    const CharacterGlyph& g = it->second;

    float pos_x = *x + g.bearing.x * scale;
    float pos_y = y - (g.size.y - g.bearing.y) * scale;
    float size_x = g.size.x * scale;
    float size_y = g.size.y * scale;

    float buf[4][4] = {
        {pos_x, pos_y + size_y, g.uv_0.x, g.uv_1.y},          // bottom-left
        {pos_x, pos_y, g.uv_0.x, g.uv_0.y},                   // top-left
        {pos_x + size_x, pos_y, g.uv_1.x, g.uv_0.y},          // top-right
        {pos_x + size_x, pos_y + size_y, g.uv_1.x, g.uv_1.y}  // bottom-right
    };

    std::copy(&buf[0][0], &buf[0][0] + 16, vertices.begin());

    *x += g.advance * scale;
}

const std::string& FontAsset::name() const {
    return m_name;
}

AssetID FontAsset::texture_id() const {
    return m_texture;
}

std::ostream& FontAsset::print(std::ostream& os) const {
    return os << "FontAsset(name: " << m_name << ", chars_num: " << m_chars.size() << ")";
}
