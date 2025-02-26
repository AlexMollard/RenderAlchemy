#include "Geometry.h"

#include <cmath>
#include <iostream>

#include "GLUtils.h"

Geometry::Geometry()
      : VAO(0), VBO(0), EBO(0), indexCount(0)
{
}

Geometry::~Geometry()
{
	cleanup();
}

void Geometry::cleanup()
{
	if (VAO != 0)
	{
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
	}

	if (VBO != 0)
	{
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}

	if (EBO != 0)
	{
		glDeleteBuffers(1, &EBO);
		EBO = 0;
	}
}

void Geometry::createCube()
{
	cleanup();

	// Create a cube with colors and normals
    float vertices[] = {
        // Positions          // Colors                 // Normals
        -0.5f, -0.5f, -0.5f,  0.5f, 0.1f, 0.1f,         0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.6f, 0.2f, 0.1f,         0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.7f, 0.3f, 0.1f,         0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.5f, 0.4f, 0.1f,         0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.1f, 0.5f, 0.1f,         0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.1f, 0.6f, 0.2f,         0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.1f, 0.7f, 0.3f,         0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.1f, 0.5f, 0.4f,         0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f,  0.1f, 0.1f, 0.7f,        -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.1f, 0.1f, 0.8f,        -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.1f, 0.1f, 0.9f,        -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.1f, 0.1f, 0.6f,        -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  0.7f, 0.7f, 0.1f,         1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.8f, 0.8f, 0.1f,         1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.9f, 0.9f, 0.1f,         1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.7f, 0.6f, 0.1f,         1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.7f, 0.1f, 0.7f,         0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.8f, 0.1f, 0.8f,         0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.9f, 0.1f, 0.9f,         0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.6f, 0.1f, 0.6f,         0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.3f, 0.3f, 0.3f,         0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.4f, 0.4f, 0.4f,         0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,         0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.2f, 0.2f, 0.2f,         0.0f,  1.0f,  0.0f
    };

    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

	indexCount = sizeof(indices) / sizeof(indices[0]);

	// Create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Bind the VAO
	glBindVertexArray(VAO);

	// Bind and set vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Bind and set element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(0);

	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Unbind the VAO
	glBindVertexArray(0);
}

void Geometry::createSphere(int xSegments, int ySegments)
{
	cleanup();

	const float PI = 3.14159265359f;

	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	// Generate sphere vertices
	for (int y = 0; y <= ySegments; y++)
	{
		for (int x = 0; x <= xSegments; x++)
		{
			float xSegment = (float) x / (float) xSegments;
			float ySegment = (float) y / (float) ySegments;
			float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
			float yPos = std::cos(ySegment * PI);
			float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

			// Position
			vertices.push_back(xPos);
			vertices.push_back(yPos);
			vertices.push_back(zPos);

			// Color (based on position for varied HDR values)
			vertices.push_back(std::abs(xPos) * 0.5f + 0.5f);
			vertices.push_back(std::abs(yPos) * 0.5f + 0.5f);
			vertices.push_back(std::abs(zPos) * 0.5f + 0.5f);

			// Normal (same as position for a sphere at origin)
			vertices.push_back(xPos);
			vertices.push_back(yPos);
			vertices.push_back(zPos);
		}
	}

	// Generate indices
	for (int y = 0; y < ySegments; y++)
	{
		for (int x = 0; x < xSegments; x++)
		{
			unsigned int current = y * (xSegments + 1) + x;
			unsigned int next = current + 1;
			unsigned int nextRow = (y + 1) * (xSegments + 1) + x;
			unsigned int nextRowNext = nextRow + 1;

			// Two triangles per quad
			indices.push_back(current);
			indices.push_back(nextRow);
			indices.push_back(next);

			indices.push_back(next);
			indices.push_back(nextRow);
			indices.push_back(nextRowNext);
		}
	}

	indexCount = indices.size();

	// Create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Bind the VAO
	glBindVertexArray(VAO);

	// Bind and set vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	// Bind and set element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(0);

	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Unbind the VAO
	glBindVertexArray(0);
}

void Geometry::createPlane(float size)
{
	cleanup();

	// Simple plane for ground
    float vertices[] = {
        // Positions          // Colors                 // Normals
        -size, 0.0f, -size,   0.8f, 0.8f, 0.8f,         0.0f, 1.0f, 0.0f,
         size, 0.0f, -size,   0.8f, 0.8f, 0.8f,         0.0f, 1.0f, 0.0f,
         size, 0.0f,  size,   0.8f, 0.8f, 0.8f,         0.0f, 1.0f, 0.0f,
        -size, 0.0f,  size,   0.8f, 0.8f, 0.8f,         0.0f, 1.0f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3
    };

	indexCount = sizeof(indices) / sizeof(indices[0]);

	// Create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Bind the VAO
	glBindVertexArray(VAO);

	// Bind and set vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Bind and set element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(0);

	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) (6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Unbind the VAO
	glBindVertexArray(0);
}

void Geometry::createScreenQuad()
{
	cleanup();

	// Fullscreen quad for post-processing
    float quadVertices[] = {
        // positions        // texCoords
        -1.0f,  1.0f,      0.0f, 1.0f,
        -1.0f, -1.0f,      0.0f, 0.0f,
         1.0f, -1.0f,      1.0f, 0.0f,
         1.0f,  1.0f,      1.0f, 1.0f
    };

    unsigned int quadIndices[] = {
        0, 1, 2,
        0, 2, 3
    };

	indexCount = 6; // 6 indices for 2 triangles

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

	// Position attribute (location 0)
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(0);

	// Texture coordinates (location 1)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Unbind VAO
	glBindVertexArray(0);
}

void Geometry::draw() const
{
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
