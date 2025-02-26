#pragma once

#include <GL/glew.h>
#include <string>

class GLUtils
{
public:
	// Debug function to check for OpenGL errors
	static bool checkGLError(const char* operation);

	// Initialize GLEW
	static bool initGLEW();

	static std::string readFile(const char* filename);
};
