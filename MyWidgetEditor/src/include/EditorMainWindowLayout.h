#pragma once
#include <imgui.h>
#include <vector>
#include "Widget.h"
#include <GLFW/glfw3.h>

void ShowMainWindowLayout(bool* p_open, bool* use_grid , ImGuiViewport* viewport, GLFWwindow* window, std::vector<Widget>& assets, std::vector<Widget>& used_assets, ImGuiIO& io);
