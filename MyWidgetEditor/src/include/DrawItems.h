#pragma once
#include <vector>
#include <Widget.h>

void DrawItems(bool* use_grid, std::vector<Widget>& used_assets, ImDrawList* draw_list, ImVec2& canvas_minp_actual,ImVec2& canvas_pmin, ImVec2& canvas_pmax);
