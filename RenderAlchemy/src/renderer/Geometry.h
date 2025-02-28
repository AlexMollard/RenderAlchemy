#pragma once

#include <bgfx/bgfx.h>
#include <vector>

class Shader; // Forward declaration

class Geometry {
public:
    Geometry();
    ~Geometry();

    // Create primitive shapes
    void createCube();
    void createSphere(int segments = 20);
    void createPlane(float size = 10.0f);
    void createScreenQuad();
    
    // Draw geometry with the specified shader
    void draw(const Shader& shader) const;
    
    // Draw geometry with custom state
    void draw(bgfx::ProgramHandle program, uint64_t state = BGFX_STATE_DEFAULT) const;
    
    // Get handle to vertex buffer
    bgfx::VertexBufferHandle getVBO() const { return vbo; }
    
    // Get handle to index buffer
    bgfx::IndexBufferHandle getIBO() const { return ibo; }

private:
    // Helper to create geometry from vertex and index data
    template<typename T>
    void createGeometry(const std::vector<T>& vertices, const std::vector<uint16_t>& indices);
    
    // Clean up resources
    void cleanup();

    bgfx::VertexBufferHandle vbo;
    bgfx::IndexBufferHandle ibo;
    bgfx::VertexLayout layout;
    uint32_t vertexCount;
    uint32_t indexCount;
    uint8_t viewId;
};
