#pragma once
#include <imgui.h>
#include <GLFW/glfw3.h>
#include "../managers/RuntimeWidgetManager.h"


namespace rn {

	static class MainWindowLayout {
	public:
		RuntimeWidgetManager widget_manager;


		static void Render(bool* p_open, ImGuiViewport* viewport, GLFWwindow* window);				

	};
}