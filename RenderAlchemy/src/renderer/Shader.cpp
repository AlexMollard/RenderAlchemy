#include "Shader.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <bx/readerwriter.h>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    compileShader(vertexPath, "vertex.bin");
    compileShader(fragmentPath, "fragment.bin");

    bgfx::ShaderHandle vsh = loadShader("vertex.bin");
    bgfx::ShaderHandle fsh = loadShader("fragment.bin");

    m_program = bgfx::createProgram(vsh, fsh, true);
}

Shader::~Shader() {
    bgfx::destroy(m_program);
    for (auto& pair : m_uniforms) {
        bgfx::destroy(pair.second);
    }
}

void Shader::setUniform(const std::string& name, const void* value, uint16_t num) {
    bgfx::setUniform(getUniform(name), value, num);
}

void Shader::setTexture(const std::string& name, bgfx::TextureHandle texture, uint8_t stage)
{
	bgfx::setTexture(stage, getUniform(name), texture);
}

static const bgfx::Memory* loadMem(bx::FileReaderI* _reader, const bx::FilePath& _filePath)
{
	if (bx::open(_reader, _filePath))
	{
		uint32_t size = (uint32_t) bx::getSize(_reader);
		const bgfx::Memory* mem = bgfx::alloc(size + 1);
		bx::read(_reader, mem->data, size, bx::ErrorAssert{});
		bx::close(_reader);
		mem->data[mem->size - 1] = '\0';
		return mem;
	}

	std::cout << "Failed to load" << _filePath.getCPtr() << std::endl;
	return NULL;
}


static void* loadMem(bx::FileReaderI* _reader, bx::AllocatorI* _allocator, const bx::FilePath& _filePath, uint32_t* _size)
{
	if (bx::open(_reader, _filePath))
	{
		uint32_t size = (uint32_t) bx::getSize(_reader);
		void* data = bx::alloc(_allocator, size);
		bx::read(_reader, data, size, bx::ErrorAssert{});
		bx::close(_reader);

		if (NULL != _size)
		{
			*_size = size;
		}
		return data;
	}

	std::cout << "Failed to load" << _filePath.getCPtr() << std::endl;
	return NULL;
}


static bgfx::ShaderHandle loadShader(bx::FileReaderI* _reader, const bx::StringView& _name)
{
	bx::FilePath filePath("shaders/");

	switch (bgfx::getRendererType())
	{
		case bgfx::RendererType::Noop:
		case bgfx::RendererType::Direct3D11:
		case bgfx::RendererType::Direct3D12:
			filePath.join("dx11");
			break;
		case bgfx::RendererType::Agc:
		case bgfx::RendererType::Gnm:
			filePath.join("pssl");
			break;
		case bgfx::RendererType::Metal:
			filePath.join("metal");
			break;
		case bgfx::RendererType::Nvn:
			filePath.join("nvn");
			break;
		case bgfx::RendererType::OpenGL:
			filePath.join("glsl");
			break;
		case bgfx::RendererType::OpenGLES:
			filePath.join("essl");
			break;
		case bgfx::RendererType::Vulkan:
			filePath.join("spirv");
			break;

		case bgfx::RendererType::Count:
			BX_ASSERT(false, "You should not be here!");
			break;
	}

	char fileName[512];
	bx::strCopy(fileName, BX_COUNTOF(fileName), _name);
	bx::strCat(fileName, BX_COUNTOF(fileName), ".bin");

	filePath.join(fileName);

	bgfx::ShaderHandle handle = bgfx::createShader(loadMem(_reader, filePath.getCPtr()));
	bgfx::setName(handle, _name.getPtr(), _name.getLength());

	return handle;
}

bgfx::ShaderHandle Shader::loadShader(const std::string& path)
{
	return bgfx::ShaderHandle();
}

void Shader::compileShader(const std::string& path, const std::string& output)
{
}

bgfx::UniformHandle Shader::getUniform(const std::string& name)
{
    auto it = m_uniforms.find(name);
    if (it != m_uniforms.end()) {
        return it->second;
    }

    bgfx::UniformHandle uniform = bgfx::createUniform(name.c_str(), bgfx::UniformType::Vec4);
    m_uniforms[name] = uniform;
    return uniform;
}
