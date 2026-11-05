#include "RuntimeWidgetManager.h"
#include <algorithm>
#include <iostream>
#include <core/Config.h>

namespace rn {

    RuntimeWidgetManager::RuntimeWidgetManager() {
        context_.is_editor = false;
        context_.live_data = true;
        context_.window = &window_props_;
        context_.bus = &bus_;
    }

    bool RuntimeWidgetManager::WindowPropsFromJson(const std::string& filename) {
        std::string error;
        if (!core::LoadWindowProperties(filename, window_props_, &error)) {
            std::cerr << error << std::endl;
            return false;
        }
        return true;
    }

    bool RuntimeWidgetManager::LoadFromFile(const std::string& filename) {
        core::ConfigData data;
        std::string error;
        if (!core::LoadConfig(filename, data, &error)) {
            std::cerr << error << std::endl;
            return false;
        }

        // Свойства окна оставляем прочитанными до создания окна: путь к вершинному шейдеру там уже задан
        widgets_ = std::move(data.widgets);
        connections_ = std::move(data.connections);

        std::vector<core::Widget*> widgets;
        for (auto& widget : widgets_) {
            widget->SetContext(&context_);
            widgets.push_back(widget.get());
        }
        bus_.SetRoutes(widgets, connections_);
        return true;
    }

    float RuntimeWidgetManager::GetDesiredFps() const {
        float fps = 1.0f;
        for (const auto& widget : widgets_) {
            fps = std::max(fps, widget->GetDesiredFps());
            if (widget->IsFading()) {
                fps = std::max(fps, 30.0f);  // плавное исчезновение/появление
            }
            else if (widget->GetFadeOnHover()) {
                fps = std::max(fps, 10.0f);  // проверяем курсор: в режиме обоев событий мыши нет
            }
        }
        return fps;
    }

    void RuntimeWidgetManager::UpdateAll(const ImVec2& origin, const ImVec2& mouse, float dt) {
        for (auto& widget : widgets_) {
            const ImVec2 min = widget->GetScreenMin(origin);
            const ImVec2 max = widget->GetScreenMax(origin);
            const bool hovered = mouse.x >= min.x && mouse.x < max.x && mouse.y >= min.y && mouse.y < max.y;
            widget->UpdateHoverFade(hovered && widget->IsVisible(), dt);
            widget->Update();
        }
        // Доставляем всё, что виджеты отправили (в том числе из фоновых потоков)
        bus_.Dispatch();
    }

    void RuntimeWidgetManager::RenderAll(ImDrawList* draw_list, const ImVec2& origin) {
        for (auto& widget : widgets_) {
            if (!widget->IsVisible()) continue;

            const int first_vertex = draw_list->VtxBuffer.Size;
            widget->Render(draw_list, widget->GetScreenMin(origin), widget->GetScreenMax(origin));

            // Прозрачность при наведении: домножаем альфу всех вершин, которые нарисовал виджет
            const float opacity = widget->GetOpacity();
            if (opacity < 1.0f) {
                for (int i = first_vertex; i < draw_list->VtxBuffer.Size; ++i) {
                    ImU32& col = draw_list->VtxBuffer[i].col;
                    const ImU32 alpha = (ImU32)(((col >> IM_COL32_A_SHIFT) & 0xFF) * opacity);
                    col = (col & ~IM_COL32_A_MASK) | (alpha << IM_COL32_A_SHIFT);
                }
            }
        }
    }

    void RuntimeWidgetManager::RenderContentAll(const ImVec2& origin) {
        for (auto& widget : widgets_) {
            if (!widget->IsVisible()) continue;

            // ImGui-элементы (таблицы, графики, поля) берут прозрачность из стиля
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * widget->GetOpacity());
            widget->RenderContent(widget->GetScreenMin(origin), widget->GetScreenMax(origin));
            ImGui::PopStyleVar();
        }
    }
}
