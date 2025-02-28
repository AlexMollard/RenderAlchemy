#pragma once

#include <bgfx/bgfx.h>
#include <map>
#include <string>
#include <vector>

// Struct to store CLUT data
class CLUT
{
public:
	CLUT();
	CLUT(const std::string& name, const std::vector<float>& data, int size, bool is3D);

	// Getters
	const std::string& getName() const;
	const std::vector<float>& getData() const;
	int getSize() const;
	bool is3DCLUT() const;

	// Setters
	void setName(const std::string& name);
	void setData(const std::vector<float>& data);
	void setSize(int size);
	void setIs3DCLUT(bool is3D);

	// Save and load CLUT data to/from a file
	void saveToFile(const std::string& filename) const;
	static CLUT loadFromFile(const std::string& filename);

	// Create bgfx texture from CLUT data
	bgfx::TextureHandle create1DTexture() const;
	bgfx::TextureHandle create3DTexture() const;

	// bgfx texture creation helpers
	static bgfx::TextureHandle create1DCLUT(const std::vector<float>& clutData, int size);
	static bgfx::TextureHandle create3DCLUT(const std::vector<float>& clutData, int size);

	// Presets
	static void createPreset1DCLUTs(std::map<std::string, CLUT>& clutLibrary);
	static void createPreset3DCLUTs(std::map<std::string, CLUT>& clutLibrary);

private:
	std::string name;
	std::vector<float> data;
	int size;
	bool is3D;
};
