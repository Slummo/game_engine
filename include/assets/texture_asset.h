#pragma once

#include "assets/iasset.h"

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <assimp/scene.h>

enum class TextureWrap : GLenum {
    Repeat = GL_REPEAT,
    ClampToEdge = GL_CLAMP_TO_EDGE,
    MirroredRepeat = GL_MIRRORED_REPEAT
};

enum class TextureFilter : GLenum {
    Nearest = GL_NEAREST,
    Linear = GL_LINEAR,
    NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
    LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
};

struct TextureParams {
    TextureWrap wrapS = TextureWrap::Repeat;
    TextureWrap wrapT = TextureWrap::Repeat;
    TextureFilter min_filter = TextureFilter::LinearMipmapLinear;
    TextureFilter mag_filter = TextureFilter::Linear;
    bool generate_mipmaps = true;
    // Whether to treat the image as sRGB for internal format (useful for color textures)
    bool srgb = false;
};

enum class TextureType {
    None = aiTextureType_NONE,
    Diffuse = aiTextureType_DIFFUSE,
    Specular = aiTextureType_SPECULAR,
    Ambient = aiTextureType_AMBIENT,
};

std::ostream& operator<<(std::ostream& os, TextureType type);

class TextureAsset : public IAsset {
public:
    TextureAsset() = default;

    static std::shared_ptr<TextureAsset> create_fallback();

    // With stb_image
    static std::optional<std::shared_ptr<TextureAsset>> load_from_file(const std::string& path, TextureType type,
                                                                       const TextureParams& params = {});
    static const char* base_path() {
        return "assets/textures/";
    }
    const std::string& full_path();

    ~TextureAsset();

    void bind(uint32_t slot = 0) const;
    static void unbind(uint32_t slot = 0);

    bool is_loaded() const;
    GLuint get_id() const;
    int32_t width() const;
    int32_t height() const;
    int32_t channels() const;
    TextureType type() const;

protected:
    std::ostream& print(std::ostream& os) const override;

private:
    GLuint m_id = 0;
    int32_t m_width = 0;
    int32_t m_height = 0;
    int32_t m_channels = 0;
    TextureType m_type = TextureType::None;
    std::string m_full_path;

    bool upload(int32_t width, int32_t height, int32_t channels, const uint8_t* data, const TextureParams& params);
};
