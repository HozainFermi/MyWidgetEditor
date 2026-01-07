#pragma once
#include <imgui.h>
#include <string>

namespace wg {

	struct RuntimeWindowProperties {	
		ImU32 bg_color= IM_COL32(40, 40, 40, 255);
		float bg_color_float[4];		
		bool always_on_top = false;
		bool moveble = true;
		std::string frag_GLSLshader_file;
		std::string vertex_GLSLshader_file;
		int width=150;
		int height=300;

	};

}
