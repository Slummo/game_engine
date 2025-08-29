#include "assets/texture.h"
#include "core/log.h"

#include <cstring>

static GLenum ToGLWrap(TextureWrap w) {
    return static_cast<GLenum>(w);
}

static GLenum ToGLFilter(TextureFilter f) {
    return static_cast<GLenum>(f);
}

std::ostream& operator<<(std::ostream& os, TextureType type) {
    switch (type) {
        case TextureType::None: {
            os << "None";
            break;
        }
        case TextureType::Diffuse: {
            os << "Diffuse";
            break;
        }
        case TextureType::Specular: {
            os << "Specular";
            break;
        }
        case TextureType::Ambient: {
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

std::optional<std::shared_ptr<Texture>> Texture::load_from_file(const std::string& path, TextureType type,
                                                                const TextureParams& params) {
    stbi_set_flip_vertically_on_load(true);
    int32_t w = 0;
    int32_t h = 0;
    int32_t channels = 0;
    uint8_t* data = stbi_load(path.c_str(), &w, &h, &channels, 0);

    if (!data) {
        ERR("[Texture] Failed to load image: " << path << " (" << stbi_failure_reason() << ")")
        return std::nullopt;
    }

    auto tex = std::make_shared<Texture>();

    bool res = tex->upload(w, h, channels, data, params);
    stbi_image_free(data);

    if (!res) {
        ERR("[Texture] Failed to upload texture: " << path);
        return std::nullopt;
    }

    tex->m_width = w;
    tex->m_height = h;
    tex->m_channels = channels;
    tex->m_full_path = std::string(path);
    tex->m_type = type;

    return tex;
}

const std::string& Texture::full_path() {
    return m_full_path;
}

std::shared_ptr<Texture> Texture::create_fallback() {
    int32_t size = 128;
    int32_t check_size = 16;
    TextureParams params{};

    const int32_t channels = 3;
    std::vector<unsigned char> pixels(size * size * channels);

    for (int32_t y = 0; y < size; ++y) {
        for (int32_t x = 0; x < size; ++x) {
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

    auto tex = std::make_shared<Texture>();
    if (!tex->upload(size, size, channels, pixels.data(), params)) {
        ERR("[Texture] Failed to upload fallback");
        return nullptr;
    }

    tex->m_type = TextureType::Diffuse;

    return tex;
}

Texture::~Texture() {
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
}

void Texture::bind(uint32_t slot) const {
    if (m_id == 0) {
        return;
    }
    GLenum act = GL_TEXTURE0 + slot;
    glActiveTexture(act);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture::unbind(uint32_t slot) {
    GLenum act = GL_TEXTURE0 + slot;
    glActiveTexture(act);
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool Texture::is_loaded() const {
    return m_id != 0;
}

GLuint Texture::get_id() const {
    return m_id;
}

int32_t Texture::width() const {
    return m_width;
}

int32_t Texture::height() const {
    return m_height;
}

int32_t Texture::channels() const {
    return m_channels;
}

TextureType Texture::type() const {
    return m_type;
}

std::ostream& Texture::print(std::ostream& os) const {
    return os << "Texture(tex_id: " << m_id << ", width: " << m_width << ", height: " << m_height
              << ", channels: " << m_channels << ", type: " << m_type << ", path: " << m_full_path << ")";
}

bool Texture::upload(int32_t width, int32_t height, int32_t channels, const uint8_t* data,
                     const TextureParams& params) {
    if (width <= 0 || height <= 0 || channels <= 0 || !data) {
        ERR("[Texture] Invalid arguments in upload");
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
        ERR("[Texture] Unsupported channel count: " << channels);
        return false;
    }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

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

    // Upload pixel data
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    // Filters & wraps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ToGLWrap(params.wrapS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ToGLWrap(params.wrapT));

    GLenum mag = ToGLFilter(params.mag_filter);
    if (mag == GL_NEAREST_MIPMAP_NEAREST || mag == GL_LINEAR_MIPMAP_LINEAR || mag == GL_NEAREST_MIPMAP_LINEAR ||
        mag == GL_LINEAR_MIPMAP_NEAREST) {
        mag = GL_LINEAR;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);

    GLenum minf = ToGLFilter(params.min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minf);

    if (params.generate_mipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}
