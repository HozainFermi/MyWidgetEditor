#pragma once
#include "glad/gl.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <span>

namespace Helpers {

	class MeshLoader {
	private:
		short attribCount=-1;
		short stride=3;

	public:
		unsigned int VAO=0;
		unsigned int VBO=0;
		unsigned int EBO=0;
		std::vector<float> points;
		std::vector<int> indices;

		MeshLoader(std::span<float> points, short stride);
		MeshLoader(unsigned int VBO, short stride, std::span<float> verticies);
		MeshLoader(unsigned int VBO, short stride, std::span<float> verticies, std::span<float> inds);
		MeshLoader(std::span<float> points, std::span<int> indices);
		MeshLoader(std::string& filepath);
		MeshLoader();		
		~MeshLoader();
		
		void AddAttribPointer(GLint size, GLsizei stride, const void* offset);
		void Draw();
		

	};

}