#include "PlotWidget.h"
#include <imgui_internal.h>
#include <iostream>

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
}

