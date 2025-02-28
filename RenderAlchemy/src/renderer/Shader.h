#pragma once

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <unordered_map>
#include <string>

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    void setUniform(const std::string& name, const void* value, uint16_t num = 1);
	void setTexture(const std::string& name, bgfx::TextureHandle texture, uint8_t stage = 0);

    bgfx::ProgramHandle m_program;
private:
    std::unordered_map<std::string, bgfx::UniformHandle> m_uniforms;

    bgfx::ShaderHandle loadShader(const std::string& path);
    void compileShader(const std::string& path, const std::string& output);
    bgfx::UniformHandle getUniform(const std::string& name);
};

