#pragma once
#include <imgui.h>
#include <string>

namespace wg {

	struct RuntimeWindowProperties {	
		ImU32 bg_color= IM_COL32(40, 40, 40, 255);
		float bg_color_float[4]{ 0.15686275f,0.15686275f ,0.15686275f ,1.0f};
		
		bool always_on_top = false;
		bool window_rounding = false;
		bool resizeble = true;
		bool mouse_passthrougth = false;
		bool moveble = true;
		bool decorated = true;

		std::string vertex_GLSLshader_file = std::string(ASSETS_SOURCE_DIR)+"/shaders/base_vertex.vert";		
		std::string frag_GLSLshader_file = "";
		int width=500;
		int height=500;
		float rounding = 0.0f;

		void FloatToImU32() {
			ImVec4 temp = {
				bg_color_float[0] * 255,
				bg_color_float[1] * 255,
				bg_color_float[2] * 255,
				bg_color_float[3] * 255
			};
			bg_color = IM_COL32(temp.x, temp.y, temp.z, temp.w);
		}

	};

}

