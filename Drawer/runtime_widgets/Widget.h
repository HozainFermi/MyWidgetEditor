#pragma once
#include <string>
#include <vector>
#include <functional>
#include <imgui.h>
#include <json.hpp>
#include <imgui_internal.h>
#include <iostream>

namespace rn {
    // Примитивный тип данных между виджетами (JSON как универсальный контейнер)
    // Строка в таблице
    using WidgetValue = nlohmann::json;


    struct PortDesc {
        std::string name;   // внутреннее имя порта
        std::string label;  // отображаемое имя
        std::string type;   // произвольный строковый тип ("event","table","number"...)
        bool is_input = true;
    };

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

        ImVec2 position_;     // Левый верхний угол относительно канваса (локальные координаты в канвасе)
        ImVec2 size_;          // Ширина и высота

        //состояние
        bool is_dragging_ = false;
        bool is_resizing_ = false;
        bool is_hovered_ = false;
        bool is_selected_ = false;
        bool stay_selected_ = false;

        //стили
        ImU32 bg_color_ = IM_COL32(40, 40, 80, 255);
        ImU32 border_color_ = IM_COL32(100, 100, 100, 255);
        ImU32 selected_border_color_ = IM_COL32(199, 197, 135, 255);
        ImU32 text_color_ = IM_COL32(255, 255, 255, 255);
        float border_thickness_ = 2.0f;

        // для ресайза
        ImVec2 drag_offset_;
        ImVec2 resize_start_size_;
        ImVec2 resize_start_pos_;

        // минимальные размеры
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
        virtual void RenderContent(ImVec2& screen_min, ImVec2& screen_max);


        // === События ===
        virtual void OnSelected() {}
        virtual void OnDeselected() {}
        virtual void OnValueChanged() {}
        virtual void OnClick() {}

        // == Порты и пайплайн (по умолчанию отсутствуют) ==
        //В коллбеке находится ссылка на OnInput связанного виджета
        using EmitCallback = std::function<void(const std::string& widget_id, const std::string& port, const std::vector<WidgetValue>& value) > ;
        std::unordered_map<std::string, EmitCallback> from_port_callbacks_;  // Храним callback'и для каждого выходного порта
        virtual std::vector<PortDesc> GetInputPorts() const { return {}; }
        virtual std::vector<PortDesc> GetOutputPorts() const { return {}; }
        virtual void OnInput(const std::string& from_widget_id, const std::string& from_port, const std::vector<WidgetValue>& value) {}

       
        // Установить callback для конкретного порта
        void SetPortCallback(const std::string& port, EmitCallback cb) {
            from_port_callbacks_[port] = std::move(cb);
        }

        // Очистить callback для порта
        void ClearPortCallback(const std::string& port) {
            from_port_callbacks_.erase(port);
        }

        // Обновленный метод Emit
        void Emit(const std::string& from_port, const std::vector<WidgetValue>& value) {
            // Ищем callback для этого порта           
            auto it = from_port_callbacks_.find(from_port);
            if (it != from_port_callbacks_.end() && it->second) {
                // Вызываем callback, передавая ID виджета, порт и значение               
                it->second(id_, from_port, value);
            }
        }
        

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

        //  
        //ImRect GetScreenRect(const ImVec2& canvas_p0) { return ImRect(canvas_p0.x + position_.x, canvas_p0.y + position_.y, canvas_p0.x + position_.x + size_.x, canvas_p0.y + position_.y + size_.y); };

    private:
        static int next_id_;        
        
    };
}