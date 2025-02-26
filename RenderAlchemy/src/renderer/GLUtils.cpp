#include "GLUtils.h"

#include <fstream>
#include <iostream>

bool GLUtils::checkGLError(const char* operation)
{
	GLenum error;
	bool hasError = false;

	while ((error = glGetError()) != GL_NO_ERROR)
	{
		std::string errorStr;
		switch (error)
		{
			case GL_INVALID_ENUM:
				errorStr = "GL_INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				errorStr = "GL_INVALID_VALUE";
				break;
			case GL_INVALID_OPERATION:
				errorStr = "GL_INVALID_OPERATION";
				break;
			case GL_STACK_OVERFLOW:
				errorStr = "GL_STACK_OVERFLOW";
				break;
			case GL_STACK_UNDERFLOW:
				errorStr = "GL_STACK_UNDERFLOW";
				break;
			case GL_OUT_OF_MEMORY:
				errorStr = "GL_OUT_OF_MEMORY";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION";
				break;
			default:
				errorStr = "UNKNOWN_ERROR";
				break;
		}
		std::cerr << "OpenGL Error after " << operation << ": " << errorStr << " (" << error << ")" << std::endl;
		hasError = true;
	}

	return !hasError;
}

bool GLUtils::initGLEW()
{
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK)
	{
		std::cerr << "Error initializing GLEW: " << glewGetErrorString(glewError) << std::endl;
		return false;
	}

	return true;
}

std::string GLUtils::readFile(const char* filename)
{
	std::string content;
	std::ifstream fileStream(filename);

	if (!fileStream.is_open())
	{
		std::cerr << "Could not read file " << filename << ". File does not exist." << std::endl;
		return "";
	}

	std::string line = "";
	while (!fileStream.eof())
	{
		std::getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();

	return content;
}
