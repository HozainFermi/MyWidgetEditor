#include "TextureContainer.hpp"
#include "stb/stb_image.h"
#include <iostream>
namespace Helpers {


TextureContainer::TextureContainer(std::string path,GLenum type,bool flipY)
{
	int width, height, nrChannels;
	
	if (flipY) {
	stbi_set_flip_vertically_on_load(true);
	}
	//D:\\repos\\shader_test\\shader_test\\shaders\\textures\\kamen.jpg
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	++counter;
	glGenTextures(1, &textures[counter]);
	glBindTexture(type, textures[counter]);

	glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (data) {
		if (path.ends_with(".jpg")) {
			glTexImage2D(type, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		if (path.ends_with(".png")) {
			glTexImage2D(type, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(type);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

void TextureContainer::AddTexture(std::string path, GLenum type, bool flipY)
{
	++counter;
	int width, height, nrChannels;

	if (flipY) {
		stbi_set_flip_vertically_on_load(true);
	}
	//D:\\repos\\shader_test\\shader_test\\shaders\\textures\\kamen.jpg
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

	glGenTextures(1, &textures[counter]);
	glBindTexture(type, textures[counter]);

	glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (data) {
		if (path.ends_with(".jpg")) {
			glTexImage2D(type, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		if (path.ends_with(".png")) {
			glTexImage2D(type, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(type);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

}

void TextureContainer::SetProperties()
{
}

void TextureContainer::ActivateAndBind()
{	
	for (unsigned int i = GL_TEXTURE0, j = 0; j <= counter; j++, i++) {
		glActiveTexture(i);
		glBindTexture(GL_TEXTURE_2D, textures[j]);
	}
}

}