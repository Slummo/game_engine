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
        case TextureKind::Font: {
            os << "Font";
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

TextureParams TextureParams::default_font_params() {
    return TextureParams{.wrap_s = TextureWrap::CLAMP_TO_EDGE,
                         .wrap_t = TextureWrap::CLAMP_TO_EDGE,
                         .min_filter = TextureFilter::LINEAR,
                         .mag_filter = TextureFilter::LINEAR,
                         .generate_mipmaps = false,
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

TextureInfo::TextureInfo(TextureKind k, TextureParams p, MaterialTextureType type) : kind(k), params(std::move(p)) {
    if (kind == TextureKind::Material) {
        data_variant = type;
    } else {
        data_variant = std::monostate{};
    }
}

TextureInfo::TextureInfo(TextureKind k) : kind(k) {
    if (kind == TextureKind::Material) {
        params = TextureParams::default_material_params();
        data_variant = MaterialTextureType::Diffuse;
    } else {
        params = TextureParams::default_font_params();
        data_variant = std::monostate{};
    }
}

TextureInfo::TextureInfo(MaterialTextureType type) : kind(TextureKind::Material) {
    params = TextureParams::default_material_params();
    data_variant = type;
}

MaterialTextureType& TextureInfo::material_type() {
    if (kind != TextureKind::Material) {
        throw std::runtime_error("[TextureInfo] Not a material texture!");
    }
    return std::get<MaterialTextureType>(data_variant);
}

const MaterialTextureType& TextureInfo::material_type() const {
    if (kind != TextureKind::Material) {
        throw std::runtime_error("[TextureInfo] Not a material texture!");
    }
    return std::get<MaterialTextureType>(data_variant);
}

std::ostream& operator<<(std::ostream& os, const TextureInfo& info) {
    os << "TextureInfo(kind: " << info.kind << ", params: " << info.params;
    if (info.kind == TextureKind::Material) {
        os << ", material_texture_type: " << info.material_type();
    }

    os << ")";

    return os;
}

std::optional<std::shared_ptr<TextureAsset>> TextureAsset::load_from_file(const std::string& path, TextureInfo info) {
    auto tex = std::make_shared<TextureAsset>();

    int32_t w = 0;
    int32_t h = 0;
    int32_t c = 0;

    if (info.kind == TextureKind::Material) {
        stbi_set_flip_vertically_on_load(true);
        uint8_t* data = stbi_load(path.c_str(), &w, &h, &c, 0);
        if (!data) {
            ERR("[TextureAsset] Failed to load image: " << path << " (" << stbi_failure_reason() << ")")
            return std::nullopt;
        }
        bool res = tex->upload(w, h, c, data, info.params, false);
        stbi_image_free(data);
        if (!res) {
            ERR("[TextureAsset] Failed to upload image texture: " << path);
            return std::nullopt;
        }
    }

    tex->m_width = w;
    tex->m_height = h;
    tex->m_channels = c;
    tex->m_full_path = std::string(path);
    tex->m_info = std::move(info);

    return tex;
}

std::optional<std::shared_ptr<TextureAsset>> TextureAsset::load_from_file(const std::string& path, TextureKind kind) {
    return TextureAsset::load_from_file(path, TextureInfo(kind));
}

std::optional<std::shared_ptr<TextureAsset>> TextureAsset::load_from_buffer(int32_t width, int32_t height,
                                                                            int32_t channels, const uint8_t* data,
                                                                            const TextureParams& params) {
    auto tex = std::make_shared<TextureAsset>();
    bool res = tex->upload(width, height, channels, data, params, true);

    if (!res) {
        ERR("[TextureAsset] Failed to upload texture");
        return std::nullopt;
    }

    tex->m_width = width;
    tex->m_height = height;
    tex->m_channels = channels;
    tex->m_full_path = std::string("nopath");
    tex->m_info = TextureInfo(TextureKind::Font, params, MaterialTextureType::None);

    return tex;
}

const std::string& TextureAsset::full_path() {
    return m_full_path;
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
    if (!tex->upload(size, size, channels, pixels.data(), tex->m_info.params, false)) {
        ERR("[TextureAsset] Failed to upload fallback");
        return nullptr;
    }

    return tex;
}

TextureAsset::~TextureAsset() {
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
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

bool TextureAsset::is_loaded() const {
    return m_id != 0;
}

uint32_t TextureAsset::get_id() const {
    return m_id;
}

int32_t TextureAsset::width() const {
    return m_width;
}

int32_t TextureAsset::height() const {
    return m_height;
}

int32_t TextureAsset::channels() const {
    return m_channels;
}

const TextureInfo& TextureAsset::info() const {
    return m_info;
}

std::ostream& TextureAsset::print(std::ostream& os) const {
    return os << "TextureAsset(tex_id: " << m_id << ", width: " << m_width << ", height: " << m_height
              << ", channels: " << m_channels << ", info: " << m_info << ", path: " << m_full_path << ")";
}

bool TextureAsset::upload(int32_t width, int32_t height, int32_t channels, const uint8_t* data,
                          const TextureParams& params, bool is_font) {
    if (width <= 0 || height <= 0 || channels <= 0 || !data) {
        ERR("[TextureAsset] Invalid arguments in upload");
        return false;
    }

    GLenum format = GL_RGB;
    GLenum internal_format = GL_RGB8;
    if (channels == 4) {
        format = GL_RGBA;
        internal_format = params.srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
    } else if (channels == 3) {
        format = GL_RGB;
        internal_format = params.srgb ? GL_SRGB8 : GL_RGB8;
    } else if (channels == 1) {
        format = GL_RED;
        internal_format = GL_R8;
    } else {
        ERR("[TextureAsset] Unsupported channel count: " << channels);
        return false;
    }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    if (is_font) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    } else {
        // Set pixel alignment based on row size
        GLint alignment = 4;
        size_t row_bytes = static_cast<size_t>(width) * static_cast<size_t>(channels);
        if ((row_bytes % 8) == 0) {
            alignment = 8;
        } else if ((row_bytes % 4) == 0) {
            alignment = 4;
        } else if ((row_bytes % 2) == 0) {
            alignment = 2;
        } else {
            alignment = 1;
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    }

    // Upload pixel data
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    // Parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLenum>(params.wrap_s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLenum>(params.wrap_t));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(params.mag_filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(params.min_filter));

    if (params.generate_mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}
