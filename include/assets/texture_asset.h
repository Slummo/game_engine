#pragma once

#include "assets/iasset.h"

#include <variant>
#include <stdexcept>
#include <memory>
#include <string>
#include <vector>
#include <optional>

#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <assimp/scene.h>

enum class TextureKind { Material, Font };
std::ostream& operator<<(std::ostream& os, TextureKind kind);

enum class MaterialTextureType {
    None = aiTextureType_NONE,
    Diffuse = aiTextureType_DIFFUSE,
    Specular = aiTextureType_SPECULAR,
    Ambient = aiTextureType_AMBIENT
};
std::ostream& operator<<(std::ostream& os, MaterialTextureType type);

enum class TextureWrap : GLenum {
    REPEAT = GL_REPEAT,
    CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
    MIRRORED_REPEAT = GL_MIRRORED_REPEAT
};
std::ostream& operator<<(std::ostream& os, TextureWrap wrap);

enum class TextureFilter : GLenum {
    NEAREST = GL_NEAREST,
    LINEAR = GL_LINEAR,
    NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
    LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR
};
std::ostream& operator<<(std::ostream& os, TextureFilter filter);

struct TextureParams {
    static TextureParams default_material_params();
    static TextureParams default_font_params();

    TextureWrap wrap_s;
    TextureWrap wrap_t;
    TextureFilter min_filter;
    TextureFilter mag_filter;
    bool generate_mipmaps;
    bool srgb;  // Whether to treat the image as sRGB for internal format
};
std::ostream& operator<<(std::ostream& os, const TextureParams& params);

struct TextureInfo {
    TextureInfo() = delete;
    TextureInfo(TextureKind k, TextureParams p, MaterialTextureType type);
    TextureInfo(TextureKind k);
    TextureInfo(MaterialTextureType type);

    TextureKind kind;
    TextureParams params;

    MaterialTextureType& material_type();
    const MaterialTextureType& material_type() const;

private:
    std::variant<std::monostate, MaterialTextureType> data_variant;
};
std::ostream& operator<<(std::ostream& os, const TextureInfo& info);

class TextureAsset : public IAsset {
public:
    static std::shared_ptr<TextureAsset> create_fallback();

    static std::optional<std::shared_ptr<TextureAsset>> load_from_file(const std::string& path, TextureInfo info);
    static std::optional<std::shared_ptr<TextureAsset>> load_from_file(const std::string& path, TextureKind kind);
    static std::optional<std::shared_ptr<TextureAsset>> load_from_buffer(int32_t width, int32_t height,
                                                                         int32_t channels, const uint8_t* data,
                                                                         const TextureParams& params);

    static const char* base_path() {
        return "assets/textures/";
    }
    const std::string& full_path();

    void bind(uint32_t slot = 0) const;
    static void unbind(uint32_t slot = 0);

    bool is_loaded() const;
    uint32_t get_id() const;
    int32_t width() const;
    int32_t height() const;
    int32_t channels() const;
    const TextureInfo& info() const;

    ~TextureAsset();

protected:
    std::ostream& print(std::ostream& os) const override;

private:
    uint32_t m_id = 0;
    int32_t m_width = 0;
    int32_t m_height = 0;
    int32_t m_channels = 0;
    TextureInfo m_info{TextureKind::Material};
    std::string m_full_path;

    bool upload(int32_t width, int32_t height, int32_t channels, const uint8_t* data, const TextureParams& params,
                bool is_font);
};
