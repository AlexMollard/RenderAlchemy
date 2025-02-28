#pragma once

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <string>
#include <vector>

class BgfxUtils {
public:
    // Initialize bgfx
    static bool init(int width, int height, void* nativeWindowHandle);
    
    // Shutdown bgfx
    static void shutdown();
    
    // Resize the viewport
    static void resize(int width, int height);
    
    // Begin frame
    static void beginFrame();
    
    // End frame and present
    static void endFrame();
    
    // Load shader from file
    static bgfx::ShaderHandle loadShader(const std::string& filePath);
    
    // Read file into a string
    static std::string readFile(const std::string& filePath);
    
    // Create vertex layout
    static bgfx::VertexLayout createVertexLayout(bool hasPosition = true, 
                                               bool hasNormal = true, 
                                               bool hasTexCoord = true, 
                                               bool hasColor = true);
    
    // Check if bgfx is initialized
    static bool isInitialized() { return bgfx::getInternalData() != nullptr; }
    
    // Convert OpenGL clear values to bgfx clear values
    static uint32_t convertClearFlags(bool color, bool depth, bool stencil);
};
