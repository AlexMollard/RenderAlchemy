#include "Shader.h"

#include <iostream>

#include "GLUtils.h"

Shader::Shader()
      : programID(0)
{
}

Shader::~Shader()
{
	if (programID != 0)
	{
		glDeleteProgram(programID);
		programID = 0;
	}
}

bool Shader::createFromStrings(const char* vertexCode, const char* fragmentCode)
{
	try
	{
		compileShader(vertexCode, fragmentCode);
		return true;
	}
	catch (const std::exception& e)
	{
		std::cerr << "ERROR::SHADER::CREATION_FAILED\n" << e.what() << std::endl;
		return false;
	}
}

bool Shader::createFromPaths(const char* vertexPath, const char* fragmentPath)
{
	std::string vertexCode;
	std::string fragmentCode;

	try
	{
		vertexCode = GLUtils::readFile(vertexPath);
		fragmentCode = GLUtils::readFile(fragmentPath);
	}
	catch (const std::exception& e)
	{
		std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ\n" << e.what() << std::endl;
		return false;
	}

	return createFromStrings(vertexCode.c_str(), fragmentCode.c_str());
}

void Shader::compileShader(const char* vertexCode, const char* fragmentCode)
{
	// Create shader program
	programID = glCreateProgram();

	if (!programID)
	{
		std::cerr << "Error creating shader program!" << std::endl;
		return;
	}

	// Create and compile vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexCode, NULL);
	glCompileShader(vertexShader);
	checkCompileErrors(vertexShader, "VERTEX");

	// Create and compile fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentCode, NULL);
	glCompileShader(fragmentShader);
	checkCompileErrors(fragmentShader, "FRAGMENT");

	// Link shaders
	glAttachShader(programID, vertexShader);
	glAttachShader(programID, fragmentShader);
	glLinkProgram(programID);
	checkCompileErrors(programID, "PROGRAM");

	// Delete shaders after linking
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void Shader::checkCompileErrors(GLuint shader, std::string type)
{
	GLint success;
	GLchar infoLog[1024];

	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			throw std::runtime_error("ERROR::SHADER_COMPILATION_ERROR of type: " + type + "\n" + infoLog);
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			throw std::runtime_error("ERROR::PROGRAM_LINKING_ERROR of type: " + type + "\n" + infoLog);
		}
	}
}

void Shader::use() const
{
	glUseProgram(programID);
}

GLuint Shader::getProgramID() const
{
	return programID;
}

void Shader::setBool(const char* name, bool value) const
{
	glUniform1i(glGetUniformLocation(programID, name), (int) value);
}

void Shader::setInt(const char* name, int value) const
{
	glUniform1i(glGetUniformLocation(programID, name), value);
}

void Shader::setFloat(const char* name, float value) const
{
	glUniform1f(glGetUniformLocation(programID, name), value);
}

void Shader::setVec2(const char* name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(programID, name), x, y);
}

void Shader::setVec3(const char* name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(programID, name), x, y, z);
}

void Shader::setVec4(const char* name, float x, float y, float z, float w) const
{
	glUniform4f(glGetUniformLocation(programID, name), x, y, z, w);
}

void Shader::setMat4(const char* name, const float* value) const
{
	glUniformMatrix4fv(glGetUniformLocation(programID, name), 1, GL_FALSE, value);
}

GLint Shader::getUniformLocation(const char* name) const
{
	return glGetUniformLocation(programID, name);
}
