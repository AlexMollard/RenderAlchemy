#pragma once

#include <bgfx/bgfx.h>

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();

    // Create framebuffer with specified dimensions
    void create(int width, int height, bool useHDR);
    
    // Resize framebuffer
    void resize(int width, int height);
    
    // Bind framebuffer for rendering
    void bind() const;
    
    // Unbind framebuffer (return to default)
    void unbind() const;
    
    // Cleanup resources
    void cleanup();
    
    // Get color texture handle
    bgfx::TextureHandle getColorTexture() const { return colorTexture; }
    
    // Get framebuffer handle
    bgfx::FrameBufferHandle getHandle() const { return framebufferHandle; }
    
    // Get dimensions
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    bgfx::FrameBufferHandle framebufferHandle;
    bgfx::TextureHandle colorTexture;
    bgfx::TextureHandle depthTexture;
    int width;
    int height;
    bool useHDR;
    uint8_t viewId;

    static uint8_t nextViewId;
};
