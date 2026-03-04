#pragma once
#include <string>
#include "glad/gl.h"
#include <string>
#include <glm.hpp>

namespace Helpers {

	class TextureContainer {
	public:
		GLuint textures[31];
		unsigned int counter=-1;		
		
		TextureContainer()= default;
		TextureContainer(std::string path,GLenum type,bool flipY);
		
		void AddTexture(std::string path, GLenum type, bool flipY);		
		void SetProperties();
		void ActivateAndBind();
		
		

		

	};

}