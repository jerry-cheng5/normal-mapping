#pragma once

#include <vector>
#include <string>

class Texture 
{
public:
	void setupTexture(const char* texturePath);
    void setupTextureCubemap(const std::vector<std::string>& texPaths);

	void bind(unsigned int slot) const;
	void unbind() const;

private:
	unsigned int ID;
	int Width, Height, BPP;
};
