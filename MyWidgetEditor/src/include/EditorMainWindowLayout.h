#pragma once
#include <imgui.h>
#include <vector>
#include "Widget.h"

void ShowMainWindowLayout(bool* p_open, bool* use_grid , ImGuiViewport* viewport, std::vector<Widget>& assets, std::vector<Widget>& used_assets, ImGuiIO& io);
