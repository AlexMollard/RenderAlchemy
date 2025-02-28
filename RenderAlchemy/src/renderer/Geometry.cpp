#include "Geometry.h"

#include <cmath>
#include <iostream>
#include <bgfx/bgfx.h>
#include "Shader.h"

// Define vertex structure for bgfx
struct PosColorNormal {
    float x, y, z;       // Position
    float r, g, b;       // Color
    float nx, ny, nz;    // Normal

    static void init(bgfx::VertexLayout& layout) {
        layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
            .end();
    }
};

// Define vertex structure for screen quads
struct PosTexCoord {
    float x, y;          // Position
    float u, v;          // Texture coordinates

    static void init(bgfx::VertexLayout& layout) {
        layout
            .begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();
    }
};

Geometry::Geometry()
    : vbo(BGFX_INVALID_HANDLE), ibo(BGFX_INVALID_HANDLE), 
      vertexCount(0), indexCount(0), viewId(0)
{
}

Geometry::~Geometry()
{
    cleanup();
}

void Geometry::cleanup()
{
    if (bgfx::isValid(vbo)) {
        bgfx::destroy(vbo);
        vbo = BGFX_INVALID_HANDLE;
    }

    if (bgfx::isValid(ibo)) {
        bgfx::destroy(ibo);
        ibo = BGFX_INVALID_HANDLE;
    }
}

template<typename T>
void Geometry::createGeometry(const std::vector<T>& vertices, const std::vector<uint16_t>& indices)
{
    cleanup();
    
    // Create vertex buffer
    const bgfx::Memory* vertexMemory = bgfx::copy(vertices.data(), sizeof(T) * vertices.size());
    vbo = bgfx::createVertexBuffer(vertexMemory, layout);
    vertexCount = static_cast<uint32_t>(vertices.size());
    
    // Create index buffer
    const bgfx::Memory* indexMemory = bgfx::copy(indices.data(), sizeof(uint16_t) * indices.size());
    ibo = bgfx::createIndexBuffer(indexMemory);
    indexCount = static_cast<uint32_t>(indices.size());
}

void Geometry::createCube()
{
    cleanup();

    // Initialize vertex layout
    PosColorNormal::init(layout);

    // Create a cube with colors and normals
    std::vector<PosColorNormal> vertices = {
        // Front face (z = -0.5)
        { -0.5f, -0.5f, -0.5f,  0.5f, 0.1f, 0.1f,  0.0f,  0.0f, -1.0f },
        {  0.5f, -0.5f, -0.5f,  0.6f, 0.2f, 0.1f,  0.0f,  0.0f, -1.0f },
        {  0.5f,  0.5f, -0.5f,  0.7f, 0.3f, 0.1f,  0.0f,  0.0f, -1.0f },
        { -0.5f,  0.5f, -0.5f,  0.5f, 0.4f, 0.1f,  0.0f,  0.0f, -1.0f },

        // Back face (z = 0.5)
        { -0.5f, -0.5f,  0.5f,  0.1f, 0.5f, 0.1f,  0.0f,  0.0f,  1.0f },
        {  0.5f, -0.5f,  0.5f,  0.1f, 0.6f, 0.2f,  0.0f,  0.0f,  1.0f },
        {  0.5f,  0.5f,  0.5f,  0.1f, 0.7f, 0.3f,  0.0f,  0.0f,  1.0f },
        { -0.5f,  0.5f,  0.5f,  0.1f, 0.5f, 0.4f,  0.0f,  0.0f,  1.0f },

        // Left face (x = -0.5)
        { -0.5f,  0.5f,  0.5f,  0.1f, 0.1f, 0.7f, -1.0f,  0.0f,  0.0f },
        { -0.5f,  0.5f, -0.5f,  0.1f, 0.1f, 0.8f, -1.0f,  0.0f,  0.0f },
        { -0.5f, -0.5f, -0.5f,  0.1f, 0.1f, 0.9f, -1.0f,  0.0f,  0.0f },
        { -0.5f, -0.5f,  0.5f,  0.1f, 0.1f, 0.6f, -1.0f,  0.0f,  0.0f },

        // Right face (x = 0.5)
        {  0.5f,  0.5f,  0.5f,  0.7f, 0.7f, 0.1f,  1.0f,  0.0f,  0.0f },
        {  0.5f,  0.5f, -0.5f,  0.8f, 0.8f, 0.1f,  1.0f,  0.0f,  0.0f },
        {  0.5f, -0.5f, -0.5f,  0.9f, 0.9f, 0.1f,  1.0f,  0.0f,  0.0f },
        {  0.5f, -0.5f,  0.5f,  0.7f, 0.6f, 0.1f,  1.0f,  0.0f,  0.0f },

        // Bottom face (y = -0.5)
        { -0.5f, -0.5f, -0.5f,  0.7f, 0.1f, 0.7f,  0.0f, -1.0f,  0.0f },
        {  0.5f, -0.5f, -0.5f,  0.8f, 0.1f, 0.8f,  0.0f, -1.0f,  0.0f },
        {  0.5f, -0.5f,  0.5f,  0.9f, 0.1f, 0.9f,  0.0f, -1.0f,  0.0f },
        { -0.5f, -0.5f,  0.5f,  0.6f, 0.1f, 0.6f,  0.0f, -1.0f,  0.0f },

        // Top face (y = 0.5)
        { -0.5f,  0.5f, -0.5f,  0.3f, 0.3f, 0.3f,  0.0f,  1.0f,  0.0f },
        {  0.5f,  0.5f, -0.5f,  0.4f, 0.4f, 0.4f,  0.0f,  1.0f,  0.0f },
        {  0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,  0.0f,  1.0f,  0.0f },
        { -0.5f,  0.5f,  0.5f,  0.2f, 0.2f, 0.2f,  0.0f,  1.0f,  0.0f }
    };

    std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0,       // Front face
        4, 5, 6, 6, 7, 4,       // Back face
        8, 9, 10, 10, 11, 8,    // Left face
        12, 13, 14, 14, 15, 12, // Right face
        16, 17, 18, 18, 19, 16, // Bottom face
        20, 21, 22, 22, 23, 20  // Top face
    };

    createGeometry(vertices, indices);
}

void Geometry::createSphere(int segments)
{
    cleanup();
    
    // Initialize vertex layout
    PosColorNormal::init(layout);
    
    const float PI = 3.14159265359f;
    
    std::vector<PosColorNormal> vertices;
    std::vector<uint16_t> indices;
    
    // Generate sphere vertices
    for (int y = 0; y <= segments; y++) {
        for (int x = 0; x <= segments; x++) {
            float xSegment = static_cast<float>(x) / static_cast<float>(segments);
            float ySegment = static_cast<float>(y) / static_cast<float>(segments);
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            
            PosColorNormal vertex;
            // Position
            vertex.x = xPos;
            vertex.y = yPos;
            vertex.z = zPos;
            
            // Color (based on position for varied HDR values)
            vertex.r = std::abs(xPos) * 0.5f + 0.5f;
            vertex.g = std::abs(yPos) * 0.5f + 0.5f;
            vertex.b = std::abs(zPos) * 0.5f + 0.5f;
            
            // Normal (same as position for a sphere at origin)
            vertex.nx = xPos;
            vertex.ny = yPos;
            vertex.nz = zPos;
            
            vertices.push_back(vertex);
        }
    }
    
    // Generate indices
    for (int y = 0; y < segments; y++) {
        for (int x = 0; x < segments; x++) {
            uint16_t current = static_cast<uint16_t>(y * (segments + 1) + x);
            uint16_t next = current + 1;
            uint16_t nextRow = static_cast<uint16_t>((y + 1) * (segments + 1) + x);
            uint16_t nextRowNext = nextRow + 1;
            
            // Two triangles per quad
            indices.push_back(current);
            indices.push_back(nextRow);
            indices.push_back(next);
            
            indices.push_back(next);
            indices.push_back(nextRow);
            indices.push_back(nextRowNext);
        }
    }
    
    createGeometry(vertices, indices);
}

void Geometry::createPlane(float size)
{
    cleanup();
    
    // Initialize vertex layout
    PosColorNormal::init(layout);
    
    // Simple plane for ground
    std::vector<PosColorNormal> vertices = {
        { -size, 0.0f, -size,  0.8f, 0.8f, 0.8f,  0.0f, 1.0f, 0.0f },
        {  size, 0.0f, -size,  0.8f, 0.8f, 0.8f,  0.0f, 1.0f, 0.0f },
        {  size, 0.0f,  size,  0.8f, 0.8f, 0.8f,  0.0f, 1.0f, 0.0f },
        { -size, 0.0f,  size,  0.8f, 0.8f, 0.8f,  0.0f, 1.0f, 0.0f }
    };
    
    std::vector<uint16_t> indices = {
        0, 1, 2,
        0, 2, 3
    };
    
    createGeometry(vertices, indices);
}

void Geometry::createScreenQuad()
{
    cleanup();
    
    // Initialize vertex layout for screen quad
    PosTexCoord::init(layout);
    
    // Fullscreen quad for post-processing
    std::vector<PosTexCoord> vertices = {
        { -1.0f,  1.0f,  0.0f, 1.0f },
        { -1.0f, -1.0f,  0.0f, 0.0f },
        {  1.0f, -1.0f,  1.0f, 0.0f },
        {  1.0f,  1.0f,  1.0f, 1.0f }
    };
    
    std::vector<uint16_t> indices = {
        0, 1, 2,
        0, 2, 3
    };
    
    const bgfx::Memory* vertexMemory = bgfx::copy(vertices.data(), sizeof(PosTexCoord) * vertices.size());
    vbo = bgfx::createVertexBuffer(vertexMemory, layout);
    vertexCount = static_cast<uint32_t>(vertices.size());
    
    const bgfx::Memory* indexMemory = bgfx::copy(indices.data(), sizeof(uint16_t) * indices.size());
    ibo = bgfx::createIndexBuffer(indexMemory);
    indexCount = static_cast<uint32_t>(indices.size());
}

void Geometry::draw(const Shader& shader) const
{
	bgfx::ProgramHandle program = shader.m_program;
    draw(program);
}

void Geometry::draw(bgfx::ProgramHandle program, uint64_t state) const
{
    if (!bgfx::isValid(vbo) || !bgfx::isValid(ibo))
        return;
        
    bgfx::setState(state);
    bgfx::setVertexBuffer(0, vbo);
    bgfx::setIndexBuffer(ibo);
    bgfx::submit(viewId, program);
}
