#pragma once

#include <GL/glew.h>
#include <vector>

class Geometry
{
public:
	Geometry();
	~Geometry();

	void createCube();
	void createSphere(int xSegments = 32, int ySegments = 32);
	void createPlane(float size = 2.0f);
	void createScreenQuad();

	void draw() const;

	// Getters
	GLuint getVAO() const
	{
		return VAO;
	}

	int getIndexCount() const
	{
		return indexCount;
	}

private:
	GLuint VAO, VBO, EBO;
	int indexCount;

	void cleanup();
};
