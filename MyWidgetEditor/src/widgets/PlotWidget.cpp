#include "PlotWidget.h"
#include <imgui_internal.h>

namespace wg {

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

    void PlotWidget::Render(ImDrawList* draw_list, const ImVec2& canvas_p0)
    {
        Widget::Render(draw_list, canvas_p0);
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
            }
            else {
                ImGui::Text("No data");
            }

            ImGui::EndChild();
        }
    }

    void PlotWidget::RenderProperties()
    {
        ImGui::Text("PlotWidget");
        ImGui::Separator();
        ImGui::Text("Inputs:");
        ImGui::BulletText("data (array of numbers)");
        ImGui::BulletText("refresh (event)");

        ImGui::Separator();
        ImGui::Text("Data source settings");
        char buf[128];
        strncpy_s(buf, column_name_.c_str(), sizeof(buf));
        if (ImGui::InputText("Column name", buf, sizeof(buf))) {
            column_name_ = buf;
        }
    }

    std::vector<PortDesc> PlotWidget::GetInputPorts() const
    {
        return {
            PortDesc("data","Data","json",true),
            PortDesc{ "refresh", "Refresh", "event", true }
        };
    }

    std::vector<PortDesc> PlotWidget::GetOutputPorts() const
    {
        return {};
    }

    void PlotWidget::OnInput(const std::string& port, const WidgetValue& value)
    {
        if (port == "data") {
            // ожидаем массив объектов [{...}], берем столбец column_name_
            y_values_.clear();
            if (value.is_array()) {
                for (const auto& row : value) {
                    if (!row.is_object()) continue;
                    if (!row.contains(column_name_)) continue;
                    const auto& cell = row[column_name_];
                    if (cell.is_number()) {
                        y_values_.push_back(cell.get<float>());
                    } else if (cell.is_string()) {
                        try {
                            y_values_.push_back(std::stof(cell.get<std::string>()));
                        } catch (...) {
                        }
                    }
                }
            }
        }
        else if (port == "refresh") {
            // на будущее: можно реагировать на событие
        }
    }

    nlohmann::json PlotWidget::ToJson() const
    {
        auto json = Widget::ToJson();
        json["column_name"] = column_name_;
        return json;
    }

    void PlotWidget::FromJson(const nlohmann::json& json)
    {
        Widget::FromJson(json);
        column_name_ = json.value("column_name", std::string("Value"));
    }
}

