#pragma once
#include <imgui.h>
#include <vector>
#include "Widget.h"

void ShowMainWindowLayout(bool* p_open, ImGuiViewport* viewport, std::vector<Widget>& widgets, ImGuiIO& io);
