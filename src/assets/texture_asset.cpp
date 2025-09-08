#include "assets/texture_asset.h"
#include "core/log.h"
#include "managers/asset_manager.h"

#include <cstring>

std::ostream& operator<<(std::ostream& os, TextureKind kind) {
    switch (kind) {
        case TextureKind::Material: {
            os << "Material";
            break;
        }
        default: {
            os << "Unknown";
            break;
        }
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, MaterialTextureType type) {
    switch (type) {
        case MaterialTextureType::None: {
            os << "None";
            break;
        }
        case MaterialTextureType::Diffuse: {
            os << "Diffuse";
            break;
        }
        case MaterialTextureType::Specular: {
            os << "Specular";
            break;
        }
        case MaterialTextureType::Ambient: {
            os << "Ambient";
            break;
        }
        default: {
            os << "Unknown";
            break;
        }
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap::REPEAT: {
            os << "REPEAT";
            break;
        }
        case TextureWrap::CLAMP_TO_EDGE: {
            os << "CLAMP_TO_EDGE";
            break;
        }
        case TextureWrap::MIRRORED_REPEAT: {
            os << "MIRRORED_REPEAT";
            break;
        }
        default: {
            os << "Unknown";
            break;
        }
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, TextureFilter filter) {
    switch (filter) {
        case TextureFilter::NEAREST: {
            os << "NEAREST";
            break;
        }
        case TextureFilter::LINEAR: {
            os << "LINEAR";
            break;
        }
        case TextureFilter::NEAREST_MIPMAP_NEAREST: {
            os << "NEAREST_MIPMAP_NEAREST";
            break;
        }
        case TextureFilter::LINEAR_MIPMAP_LINEAR: {
            os << "LINEAR_MIPMAP_LINEAR";
            break;
        }
        default: {
            os << "Unknown";
            break;
        }
    }

    return os;
}

TextureParams TextureParams::default_material_params() {
    return TextureParams{.wrap_s = TextureWrap::REPEAT,
                         .wrap_t = TextureWrap::REPEAT,
                         .min_filter = TextureFilter::LINEAR_MIPMAP_LINEAR,
                         .mag_filter = TextureFilter::LINEAR,
                         .generate_mipmaps = true,
                         .srgb = false};
}

std::ostream& operator<<(std::ostream& os, const TextureParams& params) {
    os << "TextureParams("
       << "wrap_s: " << params.wrap_s << ", "
       << "wrap_t: " << params.wrap_t << ", "
       << "min_filter: " << params.min_filter << ", "
       << "mag_filter: " << params.mag_filter << ", "
       << "generate_mipmaps: " << (params.generate_mipmaps ? "true" : "false") << ", "
       << "srgb: " << (params.srgb ? "true" : "false") << ")";
    return os;
}

TextureInfo::TextureInfo(TextureKind k, MaterialTextureType type, TextureParams p, int32_t width, int32_t height,
                         int32_t channels, std::string path)
    : kind(k),
      type(type),
      params(std::move(p)),
      width(width),
      height(height),
      channels(channels),
      full_path(std::move(path)) {
}

std::ostream& operator<<(std::ostream& os, const TextureInfo& info) {
    os << "TextureInfo(kind: " << info.kind << ", params: " << info.params;
    if (info.kind == TextureKind::Material) {
        os << ", material_texture_type: " << info.type;
    }

    os << ", width: " << info.width << ", height: " << info.height << ", channels: " << info.channels;

    if (!info.full_path.empty()) {
        os << ", full_path: " << info.full_path;
    }

    os << ")";

    return os;
}

std::optional<std::shared_ptr<TextureAsset>> TextureAsset::load_from_file(const std::string& path,
                                                                          MaterialTextureType mat_type,
                                                                          const TextureParams& params) {
    auto tex = std::make_shared<TextureAsset>();

    int32_t w = 0;
    int32_t h = 0;
    int32_t c = 0;

    stbi_set_flip_vertically_on_load(true);
    uint8_t* data = stbi_load(path.c_str(), &w, &h, &c, 0);
    if (!data) {
        ERR("[TextureAsset] Failed to load image: " << path << " (" << stbi_failure_reason() << ")")
        return std::nullopt;
    }
    tex->m_info = TextureInfo(TextureKind::Material, mat_type, params, w, h, c, std::string(path));
    bool res = tex->upload(data);
    stbi_image_free(data);
    if (!res) {
        ERR("[TextureAsset] Failed to upload texture from path: " << path);
        return std::nullopt;
    }

    return tex;
}

std::shared_ptr<TextureAsset> TextureAsset::create_fallback() {
    int32_t size = 128;
    int32_t check_size = 16;

    const int32_t channels = 3;
    std::vector<uint8_t> pixels(size * size * channels);

    for (int32_t y = 0; y < size; y++) {
        for (int32_t x = 0; x < size; x++) {
            bool c = ((x / check_size) & 1) ^ ((y / check_size) & 1);
            int32_t idx = (y * size + x) * channels;
            if (c) {
                // fuchsia
                pixels[idx + 0] = 255;  // R
                pixels[idx + 1] = 0;    // G
                pixels[idx + 2] = 255;  // B
            } else {
                // black
                pixels[idx + 0] = 0;
                pixels[idx + 1] = 0;
                pixels[idx + 2] = 0;
            }
        }
    }

    auto tex = std::make_shared<TextureAsset>();
    tex->m_info = TextureInfo(TextureKind::Material, MaterialTextureType::Diffuse,
                              TextureParams::default_material_params(), size, size, channels, "");
    if (!tex->upload(pixels.data())) {
        ERR("[TextureAsset] Failed to upload fallback");
        return nullptr;
    }

    return tex;
}

void TextureAsset::bind(uint32_t slot) const {
    if (m_id == 0) {
        return;
    }
    GLenum act = GL_TEXTURE0 + slot;
    glActiveTexture(act);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void TextureAsset::unbind(uint32_t slot) {
    GLenum act = GL_TEXTURE0 + slot;
    glActiveTexture(act);
    glBindTexture(GL_TEXTURE_2D, 0);
}

uint32_t TextureAsset::get_id() const {
    return m_id;
}

const TextureInfo& TextureAsset::info() const {
    return m_info;
}

TextureAsset::~TextureAsset() {
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
}

std::ostream& TextureAsset::print(std::ostream& os) const {
    return os << "TextureAsset(tex_id: " << m_id << ", info: " << m_info << ")";
}

bool TextureAsset::upload(const uint8_t* data) {
    GLenum format = GL_RGB;
    GLenum internal_format = GL_RGB8;
    if (m_info.channels == 4) {
        format = GL_RGBA;
        internal_format = m_info.params.srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
    } else if (m_info.channels == 3) {
        format = GL_RGB;
        internal_format = m_info.params.srgb ? GL_SRGB8 : GL_RGB8;
    } else if (m_info.channels == 1) {
        format = GL_RED;
        internal_format = GL_R8;
    } else {
        ERR("[TextureAsset] Unsupported channel count: " << m_info.channels);
        return false;
    }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    // Save and restore previous alignment
    GLint prev_align = 0;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &prev_align);

    // Compute optimal alignment based on row byte size
    size_t row_bytes = static_cast<size_t>(m_info.width) * static_cast<size_t>(m_info.channels);
    GLint alignment = (row_bytes % 8 == 0) ? 8 : (row_bytes % 4 == 0) ? 4 : (row_bytes % 2 == 0) ? 2 : 1;
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

    // Upload pixel data
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_info.width, m_info.height, 0, format, GL_UNSIGNED_BYTE, data);

    // Parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLenum>(m_info.params.wrap_s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLenum>(m_info.params.wrap_t));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(m_info.params.mag_filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(m_info.params.min_filter));

    if (m_info.params.generate_mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, prev_align);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}
