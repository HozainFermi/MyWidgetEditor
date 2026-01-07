#pragma once
#include <string>
#include <imgui.h>
#include <json.hpp>
#include <imgui_internal.h>

namespace rn {

    enum class WidgetType {
        TEXT,
        INPUT_TEXT,
        BUTTON,
        SLIDER,
        CHECKBOX,
        COMBOBOX,
        COLOR_PICKER,
        PLOT,
        TABLE,
        IMAGE,
        CONTAINER,
        CUSTOM
    };

    class Widget {
    protected:
        std::string id_;
        std::string name_;
        WidgetType type_;
        std::string widget_class_ = "None";

        ImVec2 position_;      // Левый верхний угол относительно окна
        ImVec2 size_;          // Ширина и высота

        // Состояние
        bool is_dragging_ = false;
        bool is_resizing_ = false;
        bool is_hovered_ = false;
        bool is_selected_ = false;
        bool stay_selected_ = false;

        // Стиль
        ImU32 bg_color_ = IM_COL32(40, 40, 80, 255);
        ImU32 border_color_ = IM_COL32(100, 100, 100, 255);
        ImU32 selected_border_color_ = IM_COL32(199, 197, 135, 255);
        ImU32 text_color_ = IM_COL32(255, 255, 255, 255);
        float border_thickness_ = 2.0f;

        // Для ресайза
        ImVec2 drag_offset_;
        ImVec2 resize_start_size_;
        ImVec2 resize_start_pos_;

        // Минимальные размеры
        float min_width_ = 30.0f;
        float min_height_ = 20.0f;

    public:
        Widget();
        Widget(const std::string& name, WidgetType type, const ImVec2& pos, const ImVec2& size);
        virtual ~Widget() = default;

        // Обработка взаимодействия 
        virtual bool UpdateInteraction(int widget_id);
        // Отрисовка 
        virtual void Render(ImDrawList* draw_list);
        // Отрисовка содержимого 
        virtual void RenderContent(ImVec2& screen_min, ImVec2& screen_max);


        // === События ===
        virtual void OnSelected() {}
        virtual void OnDeselected() {}
        virtual void OnValueChanged() {}
        virtual void OnClick() {}

        // === Сериализация ===        
        virtual void FromJson(const nlohmann::json& json);

        // === Геттеры/Сеттеры ===
        const std::string& GetId() const { return id_; }
        const std::string& GetName() const { return name_; }
        WidgetType GetType() const { return type_; }

        ImVec2 GetPosition() const { return position_; }
        ImVec2 GetSize() const { return size_; }
        ImVec2 GetScreenMin() const { return ImVec2(ImGui::GetWindowPos().x + position_.x, ImGui::GetWindowPos().y + position_.y); };
        ImVec2 GetScreenMax() const { return ImVec2(ImGui::GetWindowPos().x + position_.x + size_.x, ImGui::GetWindowPos().y + position_.y + size_.y); };
        const std::string& GetWidgetClass() const { return widget_class_; }

        void SetPosition(const ImVec2& pos) { position_ = pos; }
        void SetSize(const ImVec2& size) { size_ = size; };
        void SetName(const std::string& name) { name_ = name; }
        void SetWidgetClass(const std::string& widget_class) {
            widget_class_ = widget_class;
        }

        bool IsSelected() const { return is_selected_; }
        void SetSelected(bool selected);
        bool IsStaySelected() const { return stay_selected_; }
        void SetStaySelected(bool stay_selected) { stay_selected_ = stay_selected; }

        // === Вспомогательные методы ===
        std::string TypeToString(WidgetType type) {
            switch (type) {
            case WidgetType::TEXT: return "TEXT";
            case WidgetType::INPUT_TEXT: return "INPUT_TEXT";
            case WidgetType::BUTTON: return "BUTTON";
            case WidgetType::SLIDER: return "SLIDER";
            case WidgetType::CHECKBOX: return "CHECKBOX";
            case WidgetType::COMBOBOX: return "COMBOBOX";
            case WidgetType::COLOR_PICKER: return "COLOR_PICKER";
            case WidgetType::PLOT: return "PLOT";
            case WidgetType::TABLE: return "TABLE";
            case WidgetType::IMAGE: return "IMAGE";
            case WidgetType::CONTAINER: return "CONTAINER";
            case WidgetType::CUSTOM: return "CUSTOM";
            default: return "NONE";
            }
        }

    protected:
        // Внутренние методы для обработки
        bool ProcessDragging(const ImVec2& mouse_pos);
        bool ProcessResizing(const ImVec2& mouse_pos, bool proportional);
        void DrawCommonElements(ImDrawList* draw_list, const ImRect& screen_rect);

        // Конвертация координат
        //ImRect GetScreenRect(const ImVec2& canvas_p0) { return ImRect(canvas_p0.x + position_.x, canvas_p0.y + position_.y, canvas_p0.x + position_.x + size_.x, canvas_p0.y + position_.y + size_.y); };

    private:
        static int next_id_;
    };
}