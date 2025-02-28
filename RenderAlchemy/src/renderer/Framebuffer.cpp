#include "Framebuffer.h"

#include <iostream>
#include <vector>

#include "BgfxUtils.h"

uint8_t Framebuffer::nextViewId = 1; // View 0 is reserved for main view

Framebuffer::Framebuffer()
    : framebufferHandle(BGFX_INVALID_HANDLE),
      colorTexture(BGFX_INVALID_HANDLE),
      depthTexture(BGFX_INVALID_HANDLE),
      width(0),
      height(0),
      useHDR(true),
      viewId(0)
{
}

Framebuffer::~Framebuffer()
{
    cleanup();
}

void Framebuffer::cleanup()
{
    if (bgfx::isValid(framebufferHandle)) {
        bgfx::destroy(framebufferHandle);
        framebufferHandle = BGFX_INVALID_HANDLE;
    }
    
    // In bgfx, destroying a framebuffer also destroys its attachments,
    // so we don't need to explicitly destroy color and depth textures
    colorTexture = BGFX_INVALID_HANDLE;
    depthTexture = BGFX_INVALID_HANDLE;
}

void Framebuffer::create(int width, int height, bool useHDR)
{
    cleanup();

    this->width = width;
    this->height = height;
    this->useHDR = useHDR;
    
    // Assign a unique view ID for this framebuffer
    this->viewId = nextViewId++;
    
    // Set up texture formats
    bgfx::TextureFormat::Enum colorFormat = useHDR 
        ? bgfx::TextureFormat::RGBA16F 
        : bgfx::TextureFormat::RGBA8;
    
    // Create textures with proper flags
    uint64_t textureFlags = BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
    
    // Set up textures for the framebuffer
    bgfx::TextureHandle textures[2];
    
    // Create color attachment
    textures[0] = bgfx::createTexture2D(
        uint16_t(width),
        uint16_t(height),
        false,
        1,
        colorFormat,
        textureFlags
    );
    colorTexture = textures[0];
    
    // Create depth attachment
    textures[1] = bgfx::createTexture2D(
        uint16_t(width),
        uint16_t(height),
        false,
        1,
        bgfx::TextureFormat::D24S8,
        textureFlags
    );
    depthTexture = textures[1];
    
    // Create framebuffer from textures
    framebufferHandle = bgfx::createFrameBuffer(2, textures, true);
    
    if (!bgfx::isValid(framebufferHandle)) {
        std::cerr << "ERROR: Failed to create framebuffer!" << std::endl;
    }
    
    // Set up the view for this framebuffer
    bgfx::setViewFrameBuffer(viewId, framebufferHandle);
    bgfx::setViewRect(viewId, 0, 0, uint16_t(width), uint16_t(height));
    bgfx::setViewClear(viewId, 
                      BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 
                      0x303030ff, // Dark gray
                      1.0f,
                      0);
}

void Framebuffer::resize(int width, int height)
{
    create(width, height, useHDR); // Recreate with new dimensions
}

void Framebuffer::bind() const
{
    // In bgfx, we don't directly bind framebuffers like in OpenGL.
    // Instead, we set the current view ID and the framebuffer is associated with that view.
    bgfx::setViewFrameBuffer(viewId, framebufferHandle);
    bgfx::setViewRect(viewId, 0, 0, uint16_t(width), uint16_t(height));
    bgfx::touch(viewId); // Make sure the view is processed this frame
}

void Framebuffer::unbind() const
{
    // In bgfx, switching views is equivalent to switching framebuffers
    // No explicit unbind needed
}
