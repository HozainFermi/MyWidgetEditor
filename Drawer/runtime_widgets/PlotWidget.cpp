#include "PlotWidget.h"
#include <imgui_internal.h>
#include <iostream>
#include "../../external/implot/implot.h"

namespace rn {

    REGISTER_WIDGET(PlotWidget);

    PlotWidget::PlotWidget()
        : Widget("Plot", WidgetType::PLOT, ImVec2(20, 20), ImVec2(300, 200))
    {
        SetWidgetClass("PlotWidget");
    }

    PlotWidget::PlotWidget(const std::string& name, const ImVec2& pos)
        : Widget(name, WidgetType::PLOT, pos, ImVec2(300, 200))
    {
        SetWidgetClass("PlotWidget");
    }

    bool PlotWidget::UpdateInteraction(int widget_id)
    {
        return Widget::UpdateInteraction(widget_id);
    }

    void PlotWidget::Render(ImDrawList* draw_list)
    {
        Widget::Render(draw_list);
    }

    void PlotWidget::RenderContent(ImVec2& screen_min, ImVec2& screen_max)
    {
        ImVec2 size(screen_max.x - screen_min.x - 10.0f,
                    screen_max.y - screen_min.y - 10.0f);
        ImVec2 pos(screen_min.x + 5.0f, screen_min.y + 5.0f);

        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);

        if (ImGui::BeginChild(("##plot_" + GetId()).c_str(), size, false,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            if (!y_values_.empty()) {
                ImGui::PlotLines("##plot_lines",
                    y_values_.data(), (int)y_values_.size(), 0,
                    nullptr, FLT_MAX, FLT_MAX,
                    ImVec2(size.x - 20.0f, size.y - 20.0f));
            } else {
                ImGui::Text("No data");
            }
            ImGui::EndChild();
        }
    }

    void PlotWidget::FromJson(const nlohmann::json& json)
    {
        Widget::FromJson(json);
        column_name_ = json.value("column_name", std::string("Value"));
        y_values_.clear();
    }

    void PlotWidget::OnInput(const std::string& from_widget_id,const std::string& from_port, const std::vector<WidgetValue>& value)
    {        
        if (from_port == "data") {
            y_values_.clear();            
                for (const auto& row : value) {                    
                    if (!row.is_object()) continue;
                    if (!row.contains(column_name_)) continue;
                    const auto& cell = row[column_name_];
                    if (cell.is_number()) {
                        y_values_.push_back(cell.get<float>());
                    } else if (cell.is_string()) {
                        try {
                            y_values_.push_back(std::stof(cell.get<std::string>()));
                        } catch (...) {}
                    }
                }
            
        }
    }

    void FilledLinePlots() {       
        static double xs1[101], ys1[101], ys2[101], ys3[101];
        srand(0);
        for (int i = 0; i < 101; ++i) {
            xs1[i] = (float)i;
            ys1[i] = RandomRange(400.0, 450.0);
            ys2[i] = RandomRange(275.0, 350.0);
            ys3[i] = RandomRange(150.0, 225.0);
        }
        static bool show_lines = true;
        static bool show_fills = true;
        static float fill_ref = 0;
        static int shade_mode = 0;
        static ImPlotShadedFlags flags = 0;
        ImGui::Checkbox("Lines", &show_lines); ImGui::SameLine();
        ImGui::Checkbox("Fills", &show_fills);
        if (show_fills) {
            ImGui::SameLine();
            if (ImGui::RadioButton("To -INF", shade_mode == 0))
                shade_mode = 0;
            ImGui::SameLine();
            if (ImGui::RadioButton("To +INF", shade_mode == 1))
                shade_mode = 1;
            ImGui::SameLine();
            if (ImGui::RadioButton("To Ref", shade_mode == 2))
                shade_mode = 2;
            if (shade_mode == 2) {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(100);
                ImGui::DragFloat("##Ref", &fill_ref, 1, -100, 500);
            }
        }

        if (ImPlot::BeginPlot("Stock Prices")) {
            ImPlot::SetupAxes("Days", "Price");
            ImPlot::SetupAxesLimits(0, 100, 0, 500);
            if (show_fills) {
                ImPlotSpec spec;
                spec.Flags = flags;
                spec.FillAlpha = 0.25f;
                ImPlot::PlotShaded("Stock 1", xs1, ys1, 101, shade_mode == 0 ? -INFINITY : shade_mode == 1 ? INFINITY : fill_ref, spec);
                ImPlot::PlotShaded("Stock 2", xs1, ys2, 101, shade_mode == 0 ? -INFINITY : shade_mode == 1 ? INFINITY : fill_ref, spec);
                ImPlot::PlotShaded("Stock 3", xs1, ys3, 101, shade_mode == 0 ? -INFINITY : shade_mode == 1 ? INFINITY : fill_ref, spec);
            }
            if (show_lines) {
                ImPlot::PlotLine("Stock 1", xs1, ys1, 101);
                ImPlot::PlotLine("Stock 2", xs1, ys2, 101);
                ImPlot::PlotLine("Stock 3", xs1, ys3, 101);
            }
            ImPlot::EndPlot();
        }
    }

    template <typename T>
     inline T RandomRange(T min, T max) {
        T scale = rand() / (T)RAND_MAX;
        return min + scale * (max - min);
    }

    ImVec4 RandomColor() {
        ImVec4 col;
        col.x = RandomRange(0.0f, 1.0f);
        col.y = RandomRange(0.0f, 1.0f);
        col.z = RandomRange(0.0f, 1.0f);
        col.w = 1.0f;
        return col;
    }

}

