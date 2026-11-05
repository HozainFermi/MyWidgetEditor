#pragma once
#include <memory>
#include <string>
#include <vector>
#include <imgui.h>
#include <imgui_internal.h>
#include <core/EventBus.h>
#include <core/Ports.h>
#include <core/Widget.h>
#include <core/WidgetContext.h>
#include <core/WindowProperties.h>

namespace wg {

    enum class ResizeHandle {
        NONE = 0,
        TOP_LEFT = 1, TOP = 2, TOP_RIGHT = 3,
        LEFT = 4, RIGHT = 5,
        BOTTOM_LEFT = 6, BOTTOM = 7, BOTTOM_RIGHT = 8
    };

    // Виджет на канвасе редактора + состояние, которое нужно только редактору
    struct CanvasItem {
        std::unique_ptr<core::Widget> widget;

        bool selected = false;
        bool hovered = false;
        bool dragging = false;
        bool resizing = false;
        ResizeHandle active_handle = ResizeHandle::NONE;
        ImVec2 drag_offset{};
        ImVec2 resize_start_size{};
    };

    // Виджеты на канвасе: выделение, перетаскивание, ресайз, связи и сохранение
    class WidgetManager {
    public:
        WidgetManager();

        // === Виджеты ===
        void AddWidget(std::unique_ptr<core::Widget> widget);
        void DeleteWidget(const std::string& id);
        void ClearAll();
        void BringToFront(const std::string& id);

        size_t GetCount() const { return items_.size(); }
        core::Widget* GetWidget(const std::string& id) const;
        std::vector<core::Widget*> GetAllWidgets() const;

        // === Связи ===
        const std::vector<core::Connection>& GetConnections() const { return connections_; }
        bool AddConnection(const core::Connection& connection);
        void RemoveConnection(size_t index);

        // === Выделение ===
        core::Widget* GetSelectedWidget() const;
        bool IsSelected(const core::Widget* widget) const;
        void Select(const std::string& id);   // выделяет только этот виджет
        void DeselectAll();
        // Пока курсор над панелью свойств, клик не снимает выделение
        void SetKeepSelection(bool keep) { keep_selection_ = keep; }

        // === Кадр ===
        // Логика и данные виджетов (без мыши) - нужно на любой вкладке
        void UpdateWidgets();
        // UpdateWidgets + перетаскивание/ресайз/выделение на раскладке
        void UpdateAll(const ImVec2& canvas_p0, const ImVec2& canvas_size);
        void RenderAll(ImDrawList* draw_list, const ImVec2& canvas_p0);
        void RenderContentAll(const ImVec2& canvas_p0);

        // === Превью / живые данные ===
        bool IsLiveData() const { return context_.live_data; }
        // Последние данные на проводах - для инспектора
        const core::EventBus& GetBus() const { return bus_; }
        void SetLiveData(bool live);
        void SetWindowProperties(const core::WindowProperties* window) { context_.window = window; }

        // === Сериализация ===
        nlohmann::json ToJson(const core::WindowProperties& window) const;
        bool LoadFromFile(const std::string& filename, core::WindowProperties& window);

    private:
        void UpdateInteraction(CanvasItem& item, const ImVec2& canvas_p0, const ImVec2& canvas_size, bool click_is_mine);
        bool ProcessResizing(CanvasItem& item);
        bool KeepInsideCanvas(CanvasItem& item, const ImVec2& canvas_p0, const ImVec2& canvas_size);
        ResizeHandle GetHoveredHandle(const ImVec2& mouse_pos, const ImRect& rect) const;
        void DrawFrame(ImDrawList* draw_list, const CanvasItem& item, const ImRect& rect) const;
        CanvasItem* FindItem(const std::string& id);
        void ConnectPortsIfNeeded();

        std::vector<CanvasItem> items_;
        std::vector<core::Connection> connections_;
        core::EventBus bus_;
        core::WidgetContext context_;
        bool keep_selection_ = false;
        bool ports_dirty_ = true;
    };
}
