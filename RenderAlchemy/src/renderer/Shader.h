#pragma once

#include <GL/glew.h>
#include <string>

class Shader
{
public:
	Shader();
	~Shader();

	// Create shader from source strings
	bool createFromStrings(const char* vertexCode, const char* fragmentCode);
	bool createFromPaths(const char* vertexPath, const char* fragmentPath);

	// Use the shader program
	void use() const;

	// Get shader program ID
	GLuint getProgramID() const;

	// Uniform setters
	void setBool(const char* name, bool value) const;
	void setInt(const char* name, int value) const;
	void setFloat(const char* name, float value) const;
	void setVec2(const char* name, float x, float y) const;
	void setVec3(const char* name, float x, float y, float z) const;
	void setVec4(const char* name, float x, float y, float z, float w) const;
	void setMat4(const char* name, const float* value) const;

	GLint getUniformLocation(const char* name) const;

private:
	GLuint programID;

	// Helper functions
	void compileShader(const char* vertexCode, const char* fragmentCode);
	void checkCompileErrors(GLuint shader, std::string type);
};
