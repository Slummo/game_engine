#pragma once

#include "assets/iasset.h"

#include <stdexcept>
#include <memory>
#include <string>
#include <vector>
#include <optional>

#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <assimp/scene.h>

enum class TextureKind { None, Material, Font };
std::ostream& operator<<(std::ostream& os, TextureKind kind);

enum class MaterialTextureType {
    None = aiTextureType_NONE,
    Diffuse = aiTextureType_DIFFUSE,
    Specular = aiTextureType_SPECULAR,
    Ambient = aiTextureType_AMBIENT
};
std::ostream& operator<<(std::ostream& os, MaterialTextureType type);

enum class TextureWrap : GLenum {
    None,
    REPEAT = GL_REPEAT,
    CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
    MIRRORED_REPEAT = GL_MIRRORED_REPEAT
};
std::ostream& operator<<(std::ostream& os, TextureWrap wrap);

enum class TextureFilter : GLenum {
    None,
    NEAREST = GL_NEAREST,
    LINEAR = GL_LINEAR,
    NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
    LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR
};
std::ostream& operator<<(std::ostream& os, TextureFilter filter);

struct TextureParams {
    static TextureParams default_material_params();
    static TextureParams default_font_params();

    TextureWrap wrap_s = TextureWrap::None;
    TextureWrap wrap_t = TextureWrap::None;
    TextureFilter min_filter = TextureFilter::None;
    TextureFilter mag_filter = TextureFilter::None;
    bool generate_mipmaps;
    bool srgb;  // Whether to treat the image as sRGB for internal format
};
std::ostream& operator<<(std::ostream& os, const TextureParams& params);

struct TextureInfo {
    TextureInfo() = default;
    TextureInfo(TextureKind k, MaterialTextureType type, TextureParams p, int32_t width, int32_t height,
                int32_t channels, std::string path);

    TextureKind kind = TextureKind::None;
    MaterialTextureType type = MaterialTextureType::None;
    TextureParams params;

    int32_t width;
    int32_t height;
    int32_t channels;
    std::string full_path;
};
std::ostream& operator<<(std::ostream& os, const TextureInfo& info);

class TextureAsset : public IAsset {
public:
    static std::shared_ptr<TextureAsset> create_fallback();

    static std::optional<std::shared_ptr<TextureAsset>> load_from_file(
        const std::string& path, MaterialTextureType mat_type = MaterialTextureType::Diffuse,
        const TextureParams& params = TextureParams::default_material_params());
    static std::optional<std::shared_ptr<TextureAsset>> load_from_buffer(
        int32_t width, int32_t height, int32_t channels, const std::vector<uint8_t>& pixels,
        TextureKind kind = TextureKind::Font, MaterialTextureType type = MaterialTextureType::None,
        const TextureParams& params = TextureParams::default_font_params());

    static const char* base_path() {
        return "assets/textures/";
    }

    void bind(uint32_t slot = 0) const;
    static void unbind(uint32_t slot = 0);

    uint32_t get_id() const;
    const TextureInfo& info() const;

    ~TextureAsset();

protected:
    std::ostream& print(std::ostream& os) const override;

private:
    uint32_t m_id = 0;
    TextureInfo m_info;

    bool upload(const uint8_t* data);
};
