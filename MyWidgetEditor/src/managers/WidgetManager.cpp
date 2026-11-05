#include "WidgetManager.h"
#include <algorithm>
#include <iostream>
#include <core/Config.h>

namespace wg {

    namespace {
        const ImU32 kBorderColor = IM_COL32(100, 100, 100, 255);
        const ImU32 kSelectedBorderColor = IM_COL32(199, 197, 135, 255);
        const ImU32 kNameColor = IM_COL32(255, 255, 255, 255);
        const float kHandleSize = 4.0f;

        ImRect ScreenRect(const core::Widget& widget, const ImVec2& canvas_p0) {
            return ImRect(widget.GetScreenMin(canvas_p0), widget.GetScreenMax(canvas_p0));
        }

        // Центры 8 ручек ресайза в порядке ResizeHandle (1..8)
        void HandlePositions(const ImRect& rect, ImVec2 out[8]) {
            out[0] = rect.Min;
            out[1] = ImVec2((rect.Min.x + rect.Max.x) * 0.5f, rect.Min.y);
            out[2] = ImVec2(rect.Max.x, rect.Min.y);
            out[3] = ImVec2(rect.Min.x, (rect.Min.y + rect.Max.y) * 0.5f);
            out[4] = ImVec2(rect.Max.x, (rect.Min.y + rect.Max.y) * 0.5f);
            out[5] = ImVec2(rect.Min.x, rect.Max.y);
            out[6] = ImVec2((rect.Min.x + rect.Max.x) * 0.5f, rect.Max.y);
            out[7] = rect.Max;
        }
    }

    WidgetManager::WidgetManager() {
        context_.is_editor = true;
        context_.live_data = false;  // по умолчанию в редакторе превью
        context_.bus = &bus_;
    }

    // ================= Виджеты =================

    void WidgetManager::AddWidget(std::unique_ptr<core::Widget> widget) {
        widget->SetContext(&context_);
        CanvasItem item;
        item.widget = std::move(widget);
        items_.push_back(std::move(item));
        ports_dirty_ = true;
    }

    void WidgetManager::DeleteWidget(const std::string& id) {
        // Удаляем все связи виджета, а не только первую
        connections_.erase(
            std::remove_if(connections_.begin(), connections_.end(), [&id](const core::Connection& c) {
                return c.from.widget_id == id || c.to.widget_id == id;
            }),
            connections_.end());

        items_.erase(
            std::remove_if(items_.begin(), items_.end(), [&id](const CanvasItem& item) {
                return item.widget->GetId() == id;
            }),
            items_.end());
        ports_dirty_ = true;
    }

    void WidgetManager::ClearAll() {
        items_.clear();
        connections_.clear();
        ports_dirty_ = true;
    }

    void WidgetManager::BringToFront(const std::string& id) {
        auto it = std::find_if(items_.begin(), items_.end(), [&id](const CanvasItem& item) {
            return item.widget->GetId() == id;
        });
        if (it != items_.end()) {
            std::rotate(it, it + 1, items_.end());
        }
    }

    CanvasItem* WidgetManager::FindItem(const std::string& id) {
        for (auto& item : items_) {
            if (item.widget->GetId() == id) {
                return &item;
            }
        }
        return nullptr;
    }

    core::Widget* WidgetManager::GetWidget(const std::string& id) const {
        for (const auto& item : items_) {
            if (item.widget->GetId() == id) {
                return item.widget.get();
            }
        }
        return nullptr;
    }

    std::vector<core::Widget*> WidgetManager::GetAllWidgets() const {
        std::vector<core::Widget*> result;
        result.reserve(items_.size());
        for (const auto& item : items_) {
            result.push_back(item.widget.get());
        }
        return result;
    }

    // ================= Связи =================

    bool WidgetManager::AddConnection(const core::Connection& connection) {
        if (!core::AddUniqueConnection(connections_, connection)) {
            return false;
        }
        ports_dirty_ = true;
        return true;
    }

    void WidgetManager::RemoveConnection(size_t index) {
        if (index < connections_.size()) {
            connections_.erase(connections_.begin() + index);
            ports_dirty_ = true;
        }
    }

    void WidgetManager::ConnectPortsIfNeeded() {
        if (!ports_dirty_) {
            return;
        }
        bus_.SetRoutes(GetAllWidgets(), connections_);
        ports_dirty_ = false;
    }

    // ================= Выделение =================

    core::Widget* WidgetManager::GetSelectedWidget() const {
        for (const auto& item : items_) {
            if (item.selected) {
                return item.widget.get();
            }
        }
        return nullptr;
    }

    bool WidgetManager::IsSelected(const core::Widget* widget) const {
        for (const auto& item : items_) {
            if (item.widget.get() == widget) {
                return item.selected;
            }
        }
        return false;
    }

    void WidgetManager::Select(const std::string& id) {
        for (auto& item : items_) {
            item.selected = (item.widget->GetId() == id);
        }
    }

    void WidgetManager::DeselectAll() {
        for (auto& item : items_) {
            item.selected = false;
        }
    }

    void WidgetManager::SetLiveData(bool live) {
        if (context_.live_data == live) {
            return;
        }
        context_.live_data = live;
        for (auto& item : items_) {
            item.widget->OnContextChanged();
        }
        ports_dirty_ = true;
    }

    // ================= Кадр =================

    void WidgetManager::UpdateAll(const ImVec2& canvas_p0, const ImVec2& canvas_size) {
        ConnectPortsIfNeeded();
        for (auto& item : items_) {
            item.widget->Update();
        }
        // Доставляем всё, что виджеты отправили (в том числе из фоновых потоков)
        bus_.Dispatch();

        // Клик обрабатываем, только если он внутри канваса и не по всплывающему окну поверх него.
        // Иначе выбор виджета в левой панели сразу сбрасывался бы этим же кликом.
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const ImRect canvas(canvas_p0, ImVec2(canvas_p0.x + canvas_size.x, canvas_p0.y + canvas_size.y));
        const bool click_on_canvas = ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
            canvas.Contains(mouse) && !keep_selection_ &&
            // В этот момент активна невидимая кнопка канваса - без флага ImGui считал бы окно "не под курсором"
            ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

        // Клик получает только верхний виджет под курсором (раньше тащились все перекрытые сразу)
        int click_owner = -1;
        if (click_on_canvas) {
            for (int i = (int)items_.size() - 1; i >= 0; --i) {
                const CanvasItem& item = items_[i];
                ImRect rect = ScreenRect(*item.widget, canvas_p0);
                if (rect.Contains(mouse) || (item.selected && GetHoveredHandle(mouse, rect) != ResizeHandle::NONE)) {
                    click_owner = i;
                    break;
                }
            }
        }

        for (int i = 0; i < (int)items_.size(); ++i) {
            CanvasItem& item = items_[i];
            if (click_on_canvas && i != click_owner && item.selected && !ImGui::GetIO().KeyCtrl) {
                item.selected = false;  // клик по другому месту канваса
            }
            UpdateInteraction(item, canvas_p0, canvas_size, click_on_canvas && i == click_owner);
        }
    }

    void WidgetManager::UpdateInteraction(CanvasItem& item, const ImVec2& canvas_p0, const ImVec2& canvas_size, bool click_is_mine) {
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const ImRect rect = ScreenRect(*item.widget, canvas_p0);
        item.hovered = rect.Contains(mouse);

        if (click_is_mine) {
            ResizeHandle handle = item.selected ? GetHoveredHandle(mouse, rect) : ResizeHandle::NONE;
            if (handle != ResizeHandle::NONE) {
                item.resizing = true;
                item.active_handle = handle;
                item.resize_start_size = item.widget->GetSize();
            }
            else {
                item.dragging = true;
                item.drag_offset = ImVec2(mouse.x - rect.Min.x, mouse.y - rect.Min.y);
                item.selected = true;
            }
        }

        if (item.dragging && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            item.widget->SetPosition(ImVec2(
                mouse.x - canvas_p0.x - item.drag_offset.x,
                mouse.y - canvas_p0.y - item.drag_offset.y));
            KeepInsideCanvas(item, canvas_p0, canvas_size);
        }

        if (item.resizing && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            ProcessResizing(item);
            KeepInsideCanvas(item, canvas_p0, canvas_size);
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            item.dragging = false;
            item.resizing = false;
            item.active_handle = ResizeHandle::NONE;
        }
    }

    bool WidgetManager::KeepInsideCanvas(CanvasItem& item, const ImVec2& canvas_p0, const ImVec2& canvas_size) {
        core::Widget& widget = *item.widget;
        ImVec2 pos = widget.GetPosition();
        ImVec2 size = widget.GetSize();

        size.x = (std::min)(size.x, canvas_size.x);
        size.y = (std::min)(size.y, canvas_size.y);
        ImVec2 clamped(
            std::clamp(pos.x, 0.0f, (std::max)(0.0f, canvas_size.x - size.x)),
            std::clamp(pos.y, 0.0f, (std::max)(0.0f, canvas_size.y - size.y)));

        bool changed = clamped.x != pos.x || clamped.y != pos.y || size.x != widget.GetSize().x || size.y != widget.GetSize().y;
        widget.SetPosition(clamped);
        widget.SetSize(size);
        return changed;
    }

    ResizeHandle WidgetManager::GetHoveredHandle(const ImVec2& mouse_pos, const ImRect& rect) const {
        ImVec2 handles[8];
        HandlePositions(rect, handles);
        for (int i = 0; i < 8; i++) {
            ImRect handle_rect(handles[i].x - kHandleSize, handles[i].y - kHandleSize,
                handles[i].x + kHandleSize, handles[i].y + kHandleSize);
            if (handle_rect.Contains(mouse_pos)) {
                return static_cast<ResizeHandle>(i + 1);
            }
        }
        return ResizeHandle::NONE;
    }

    bool WidgetManager::ProcessResizing(CanvasItem& item) {
        core::Widget& widget = *item.widget;
        const ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
        const ImVec2 start = item.resize_start_size;
        const bool proportional = ImGui::GetIO().KeyShift;  // Shift - сохранять пропорции
        ImVec2 new_size = start;

        switch (item.active_handle) {
        case ResizeHandle::TOP_LEFT:
            new_size.x -= delta.x;
            new_size.y -= proportional ? delta.x : delta.y;
            break;
        case ResizeHandle::TOP:
            new_size.y -= delta.y;
            if (proportional) new_size.x = new_size.y * (start.x / start.y);
            break;
        case ResizeHandle::TOP_RIGHT:
            new_size.x += delta.x;
            new_size.y += proportional ? delta.x : -delta.y;
            break;
        case ResizeHandle::LEFT:
            new_size.x -= delta.x;
            if (proportional) new_size.y = new_size.x * (start.y / start.x);
            break;
        case ResizeHandle::RIGHT:
            new_size.x += delta.x;
            if (proportional) new_size.y = new_size.x * (start.y / start.x);
            break;
        case ResizeHandle::BOTTOM_LEFT:
            new_size.x -= delta.x;
            new_size.y += proportional ? delta.x : delta.y;
            break;
        case ResizeHandle::BOTTOM:
            new_size.y += delta.y;
            if (proportional) new_size.x = new_size.y * (start.x / start.y);
            break;
        case ResizeHandle::BOTTOM_RIGHT:
            new_size.x += delta.x;
            new_size.y += proportional ? delta.x : delta.y;
            break;
        default:
            return false;
        }

        const ImVec2 min_size = widget.GetMinSize();
        new_size.x = ImMax(new_size.x, ImMax(10.0f, min_size.x));
        new_size.y = ImMax(new_size.y, ImMax(10.0f, min_size.y));

        // Противоположная от ручки сторона остаётся на месте
        ImRect r(widget.GetPosition(), ImVec2(widget.GetPosition().x + widget.GetSize().x, widget.GetPosition().y + widget.GetSize().y));
        switch (item.active_handle) {
        case ResizeHandle::TOP_LEFT:     r.Min.x = r.Max.x - new_size.x; r.Min.y = r.Max.y - new_size.y; break;
        case ResizeHandle::TOP:          r.Min.y = r.Max.y - new_size.y; break;
        case ResizeHandle::TOP_RIGHT:    r.Max.x = r.Min.x + new_size.x; r.Min.y = r.Max.y - new_size.y; break;
        case ResizeHandle::LEFT:         r.Min.x = r.Max.x - new_size.x; break;
        case ResizeHandle::RIGHT:        r.Max.x = r.Min.x + new_size.x; break;
        case ResizeHandle::BOTTOM_LEFT:  r.Min.x = r.Max.x - new_size.x; r.Max.y = r.Min.y + new_size.y; break;
        case ResizeHandle::BOTTOM:       r.Max.y = r.Min.y + new_size.y; break;
        case ResizeHandle::BOTTOM_RIGHT: r.Max.x = r.Min.x + new_size.x; r.Max.y = r.Min.y + new_size.y; break;
        default: break;
        }
        widget.SetPosition(r.Min);
        widget.SetSize(ImVec2(r.Max.x - r.Min.x, r.Max.y - r.Min.y));
        return true;
    }

    void WidgetManager::RenderAll(ImDrawList* draw_list, const ImVec2& canvas_p0) {
        for (const auto& item : items_) {
            const ImRect rect = ScreenRect(*item.widget, canvas_p0);
            draw_list->AddRectFilled(rect.Min, rect.Max, item.widget->GetBgColor());
            DrawFrame(draw_list, item, rect);
            draw_list->AddText(ImVec2(rect.Min.x + 5, rect.Min.y + 5), kNameColor, item.widget->GetName().c_str());
            item.widget->Render(draw_list, rect.Min, rect.Max);

            // Плашка поверх содержимого: в запущенном виджете этот виджет не рисуется
            if (!item.widget->IsVisible()) {
                const char* badge = "hidden";
                const ImVec2 size = ImGui::CalcTextSize(badge);
                const ImVec2 badge_min(rect.Max.x - size.x - 12, rect.Min.y + 4);
                draw_list->AddRectFilled(badge_min, ImVec2(rect.Max.x - 4, badge_min.y + size.y + 4), IM_COL32(0, 0, 0, 180), 3.0f);
                draw_list->AddText(ImVec2(badge_min.x + 4, badge_min.y + 2), IM_COL32(255, 200, 90, 255), badge);
            }
        }
    }

    void WidgetManager::DrawFrame(ImDrawList* draw_list, const CanvasItem& item, const ImRect& rect) const {
        const ImU32 color = item.selected ? kSelectedBorderColor : kBorderColor;
        draw_list->AddRect(rect.Min, rect.Max, color, 0.0f, 0, 2.0f);

        ImVec2 handles[8];
        HandlePositions(rect, handles);
        for (const ImVec2& h : handles) {
            draw_list->AddRectFilled(ImVec2(h.x - kHandleSize, h.y - kHandleSize), ImVec2(h.x + kHandleSize, h.y + kHandleSize), color);
        }
    }

    void WidgetManager::RenderContentAll(const ImVec2& canvas_p0) {
        for (const auto& item : items_) {
            item.widget->RenderContent(item.widget->GetScreenMin(canvas_p0), item.widget->GetScreenMax(canvas_p0));
        }
    }

    // ================= Сериализация =================

    nlohmann::json WidgetManager::ToJson(const core::WindowProperties& window) const {
        std::vector<const core::Widget*> widgets;
        for (const auto& item : items_) {
            widgets.push_back(item.widget.get());
        }
        return core::ConfigToJson(window, widgets, connections_);
    }

    bool WidgetManager::LoadFromFile(const std::string& filename, core::WindowProperties& window) {
        core::ConfigData data;
        std::string error;
        if (!core::LoadConfig(filename, data, &error)) {
            std::cerr << error << std::endl;
            return false;
        }

        // Путь к вершинному шейдеру в конфиге не хранится - оставляем текущий
        std::string vertex_shader = window.vertex_GLSLshader_file;
        window = data.window;
        window.vertex_GLSLshader_file = vertex_shader;

        ClearAll();
        for (auto& widget : data.widgets) {
            AddWidget(std::move(widget));
        }
        connections_ = std::move(data.connections);
        ports_dirty_ = true;
        return true;
    }
}
