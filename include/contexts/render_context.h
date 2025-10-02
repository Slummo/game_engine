#pragma once

#include "contexts/icontext.h"
#include "core/types/id.h"
#include "core/log.h"

#include <cstdint>

#include <glad/glad.h>

class Window;

struct RenderContext : public IContext {
    RenderContext(Window& window) : win(window) {
    }

    AssetID environment_id;
    Window& win;
    uint32_t scene_panel_fbo = 0;
    uint32_t scene_panel_rbo = 0;
    uint32_t scene_panel_w = 0;
    uint32_t scene_panel_h = 0;
    uint32_t texture_id = 0;

    void create_scene_panel_fbo(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) {
            ERR("[RenderContext] Zero dimensions for custom framebuffer!");
            return;
        }

        scene_panel_w = width;
        scene_panel_h = height;

        // Create the scene_panel_fbo
        glGenFramebuffers(1, &scene_panel_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, scene_panel_fbo);

        // Create the texture
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Add the texture to the scene_panel_fbo as a color attachment
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

        // Create the scene_panel_rbo
        glGenRenderbuffers(1, &scene_panel_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, scene_panel_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, scene_panel_rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            ERR("[RenderContext] Framebuffer is not complete!");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    void rescale_scene_panel_fbo(uint32_t width, uint32_t height) {
        if ((width == 0 || height == 0) || (scene_panel_w == width && scene_panel_h == height)) {
            return;
        }

        scene_panel_w = width;
        scene_panel_h = height;

        glBindFramebuffer(GL_FRAMEBUFFER, scene_panel_fbo);

        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

        glBindRenderbuffer(GL_RENDERBUFFER, scene_panel_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, scene_panel_rbo);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
};