#pragma once

#include <GL/glew.h>

class Framebuffer
{
public:
	Framebuffer();
	~Framebuffer();

	void create(int width, int height, bool useHDR = true);
	void resize(int width, int height);
	void bind() const;
	void unbind() const;

	GLuint getColorTexture() const
	{
		return colorTexture;
	}

private:
	GLuint FBO;
	GLuint colorTexture;
	GLuint RBO;
	int width;
	int height;
	bool useHDR;

	void cleanup();
};
