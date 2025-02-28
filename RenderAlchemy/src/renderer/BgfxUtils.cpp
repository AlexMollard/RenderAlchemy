#include "BgfxUtils.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/file.h>

bool BgfxUtils::init(int width, int height, void* nativeWindowHandle) {
    if (nativeWindowHandle == nullptr) {
        std::cerr << "Error: nativeWindowHandle is null!" << std::endl;
        return false;
    }

    bgfx::PlatformData pd;
    pd.nwh = nativeWindowHandle;
    pd.ndt = nullptr; // Set to nullptr if not using X11
    pd.context = nullptr;
    pd.backBuffer = nullptr;
    pd.backBufferDS = nullptr;
    bgfx::setPlatformData(pd);

    // Log the platform data to ensure it is set correctly
    std::cout << "Platform data set with native window handle: " << nativeWindowHandle << std::endl;

    bgfx::Init init;
    init.type = bgfx::RendererType::Count; // Auto-select renderer
	init.vendorId = BGFX_PCI_ID_NONE;
	init.deviceId = 0;
    init.resolution.width = width;
    init.resolution.height = height;
    init.resolution.reset = BGFX_RESET_VSYNC;
	init.platformData = pd;

    if (!bgfx::init(init)) {
        std::cerr << "Failed to initialize bgfx!" << std::endl;
        return false;
    }

    // Enable debug text
    bgfx::setDebug(BGFX_DEBUG_TEXT | BGFX_DEBUG_STATS);

    // Set view clear state
    bgfx::setViewClear(0, 
                      BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 
                      0x303030ff, // Dark gray
                      1.0f,
                      0);

    // Set view rectangle for view 0
    bgfx::setViewRect(0, 0, 0, width, height);

    return true;
}

void BgfxUtils::shutdown() {
    bgfx::shutdown();
}

void BgfxUtils::resize(int width, int height) {
    bgfx::reset(width, height, BGFX_RESET_VSYNC);
    bgfx::setViewRect(0, 0, 0, width, height);
}

void BgfxUtils::beginFrame() {
    bgfx::touch(0);
}

void BgfxUtils::endFrame() {
    bgfx::frame();
}

bgfx::ShaderHandle BgfxUtils::loadShader(const std::string& filePath) {
    std::string shaderData = readFile(filePath);
    
    if (shaderData.empty()) {
        std::cerr << "Failed to read shader file: " << filePath << std::endl;
        return BGFX_INVALID_HANDLE;
    }
    
    const bgfx::Memory* mem = bgfx::copy(shaderData.c_str(), static_cast<uint32_t>(shaderData.size()));
    bgfx::ShaderHandle handle = bgfx::createShader(mem);
    
    if (!bgfx::isValid(handle)) {
        std::cerr << "Failed to create shader from file: " << filePath << std::endl;
        return BGFX_INVALID_HANDLE;
    }
    
    // Set shader name for debug purposes
    bgfx::setName(handle, filePath.c_str());
    return handle;
}

std::string BgfxUtils::readFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return "";
    }
    
    std::string content;
    file.seekg(0, std::ios::end);
    content.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&content[0], content.size());
    file.close();
    
    return content;
}

bgfx::VertexLayout BgfxUtils::createVertexLayout(bool hasPosition, bool hasNormal, bool hasTexCoord, bool hasColor) {
    bgfx::VertexLayout layout;
    layout.begin();
    
    if (hasPosition) {
        layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
    }
    
    if (hasNormal) {
        layout.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float);
    }
    
    if (hasTexCoord) {
        layout.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
    }
    
    if (hasColor) {
        layout.add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Float);
    }
    
    layout.end();
    return layout;
}

uint32_t BgfxUtils::convertClearFlags(bool color, bool depth, bool stencil) {
    uint32_t flags = 0;
    if (color) flags |= BGFX_CLEAR_COLOR;
    if (depth) flags |= BGFX_CLEAR_DEPTH;
    if (stencil) flags |= BGFX_CLEAR_STENCIL;
    return flags;
}
