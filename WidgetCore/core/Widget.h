#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <imgui.h>
#include <json.hpp>
#include "Ports.h"
#include "WidgetContext.h"

namespace core {

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

    const char* WidgetTypeToString(WidgetType type);

    // Базовый класс всех виджетов. Один и тот же код работает в редакторе и в Drawer:
    // виджет знает только свои данные и то, как нарисовать себя внутри прямоугольника.
    // Перетаскивание, ресайз и выделение - забота редактора.
    class Widget {
    public:
        Widget(WidgetType type, const ImVec2& default_size);
        virtual ~Widget() = default;
        Widget(const Widget&) = delete;
        Widget& operator=(const Widget&) = delete;

        // === Кадр ===
        // Логика кадра (таймеры, приём результатов фоновой загрузки). Вызывается до отрисовки
        virtual void Update() {}
        // Отрисовка через ImDrawList внутри [min, max] (экранные координаты)
        virtual void Render(ImDrawList* draw_list, const ImVec2& min, const ImVec2& max) {}
        // Обычные ImGui-элементы (таблицы, графики, поля ввода) внутри [min, max]
        virtual void RenderContent(const ImVec2& min, const ImVec2& max) {}
        // Панель свойств в редакторе
        virtual void RenderProperties() {}
        // Контекст изменился (например, в редакторе переключили превью/живые данные)
        virtual void OnContextChanged() {}
        // Узел логики (Filter, Map...): нет на раскладке окна и в Drawer, живёт только на вкладке Logic
        virtual bool IsLogicNode() const { return false; }
        // Сколько кадров в секунду нужно виджету без действий пользователя. Окно рисуется с частотой
        // самого требовательного виджета (но не выше лимита окна). Анимациям нужно ~30, статике хватит 1-2
        virtual float GetDesiredFps() const { return 2.0f; }

        // === Порты ===
        virtual std::vector<PortDesc> GetInputPorts() const { return {}; }
        virtual std::vector<PortDesc> GetOutputPorts() const { return {}; }
        // Связи только что (пере)подключены - можно заново отправить последние данные
        virtual void OnPortsConnected() {}
        // Данные пришли на входной порт to_port от виджета from_widget_id
        virtual void OnInput(const std::string& to_port, const std::string& from_widget_id, const std::vector<WidgetValue>& value) {}

        // Отправляет данные с выходного порта во все подключённые виджеты (через шину событий).
        // Можно вызывать из любого потока; OnInput у получателей сработает на главном потоке
        void Emit(const std::string& port, const std::vector<WidgetValue>& value);

        // === Сериализация ===
        virtual nlohmann::json ToJson() const;
        virtual void FromJson(const nlohmann::json& json);

        // === Контекст ===
        void SetContext(const WidgetContext* context);
        const WidgetContext& Context() const { return *context_; }
        // В редакторе с выключенными живыми данными виджеты показывают превью
        bool ShowsPreview() const { return context_->is_editor && !context_->live_data; }

        // === Геттеры/сеттеры ===
        const std::string& GetId() const { return id_; }
        const std::string& GetName() const { return name_; }
        WidgetType GetType() const { return type_; }
        const std::string& GetWidgetClass() const { return widget_class_; }

        ImVec2 GetPosition() const { return position_; }
        ImVec2 GetSize() const { return size_; }
        ImVec2 GetMinSize() const { return ImVec2(min_width_, min_height_); }
        ImU32 GetBgColor() const { return bg_color_; }
        // Экранные координаты при заданном начале координат (угол канваса или окна)
        ImVec2 GetScreenMin(const ImVec2& origin) const { return ImVec2(origin.x + position_.x, origin.y + position_.y); }
        ImVec2 GetScreenMax(const ImVec2& origin) const { return ImVec2(origin.x + position_.x + size_.x, origin.y + position_.y + size_.y); }

        void SetPosition(const ImVec2& pos) { position_ = pos; }
        void SetSize(const ImVec2& size) { size_ = size; }
        void SetName(const std::string& name) { name_ = name; }
        void SetWidgetClass(const std::string& widget_class) { widget_class_ = widget_class; }

        // === Положение блока на вкладке Logic (на окно не влияет) ===
        bool HasGraphPosition() const { return has_graph_position_; }
        ImVec2 GetGraphPosition() const { return graph_position_; }
        void SetGraphPosition(const ImVec2& pos) { graph_position_ = pos; has_graph_position_ = true; }

        // === Видимость и прозрачность при наведении (действуют в Drawer) ===
        // Невидимый виджет не рисуется, но работает: например, таблица, которая только кормит график
        bool IsVisible() const { return visible_; }
        void SetVisible(bool visible) { visible_ = visible; }
        // Под курсором виджет плавно становится прозрачнее (до fade_min_alpha), без курсора возвращается
        bool GetFadeOnHover() const { return fade_on_hover_; }
        void SetFadeOnHover(bool fade) { fade_on_hover_ = fade; }
        float GetFadeMinAlpha() const { return fade_min_alpha_; }
        void SetFadeMinAlpha(float alpha) { fade_min_alpha_ = alpha; }
        float GetFadeSpeed() const { return fade_speed_; }
        void SetFadeSpeed(float speed) { fade_speed_ = speed; }
        // Вызывать каждый кадр: hovered - курсор над виджетом, dt - секунды с прошлого кадра
        void UpdateHoverFade(bool hovered, float dt);
        // Текущая непрозрачность 0..1 (1 - обычный вид)
        float GetOpacity() const { return opacity_; }
        // Прозрачность сейчас меняется - нужна плавная анимация
        bool IsFading() const;

    protected:
        std::string id_;
        std::string name_ = "Unnamed";
        WidgetType type_;
        std::string widget_class_ = "Widget";

        ImVec2 position_{ 0.0f, 0.0f };   // левый верхний угол относительно канваса/окна
        ImVec2 size_;

        ImU32 bg_color_ = IM_COL32(40, 40, 80, 255);
        ImU32 text_color_ = IM_COL32(255, 255, 255, 255);
        float min_width_ = 30.0f;
        float min_height_ = 20.0f;

        ImVec2 graph_position_{ 0.0f, 0.0f };
        bool has_graph_position_ = false;

        bool visible_ = true;
        bool fade_on_hover_ = false;
        float fade_min_alpha_ = 0.15f;
        float fade_speed_ = 1.0f;   // на сколько меняется непрозрачность за секунду
        float opacity_ = 1.0f;      // не сохраняется

    private:
        // Новые id не должны совпадать с загруженными из конфига
        static void ReserveId(const std::string& id);
        static int next_id_;

        const WidgetContext* context_;
    };
}
