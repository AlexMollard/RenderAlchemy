#include "Framebuffer.h"

#include <iostream>

#include "GLUtils.h"

Framebuffer::Framebuffer()
      : FBO(0), colorTexture(0), RBO(0), width(0), height(0), useHDR(true)
{
}

Framebuffer::~Framebuffer()
{
	cleanup();
}

void Framebuffer::cleanup()
{
	if (FBO != 0)
	{
		glDeleteFramebuffers(1, &FBO);
		FBO = 0;
	}

	if (colorTexture != 0)
	{
		glDeleteTextures(1, &colorTexture);
		colorTexture = 0;
	}

	if (RBO != 0)
	{
		glDeleteRenderbuffers(1, &RBO);
		RBO = 0;
	}
}

void Framebuffer::create(int width, int height, bool useHDR)
{
	cleanup();

	this->width = width;
	this->height = height;
	this->useHDR = useHDR;

	// Create framebuffer
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// Create texture attachment
	glGenTextures(1, &colorTexture);
	glBindTexture(GL_TEXTURE_2D, colorTexture);

	// Choose format based on HDR setting
	if (useHDR)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

	// Create renderbuffer object for depth and stencil attachment
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "ERROR: Framebuffer is not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(int width, int height)
{
	create(width, height, useHDR); // Recreate with new dimensions
}

void Framebuffer::bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void Framebuffer::unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
