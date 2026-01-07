#pragma once
#include "../widgets/Widget.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include "../ui/RuntimeWindowProperties.h"

namespace wg {
    class WidgetManager {
    private:
        std::vector<std::unique_ptr<Widget>> widgets_;
        std::unordered_map<std::string, Widget*> widgets_by_id_;
        int selected_widget_id_ = -1;

    public:
        // === Управление виджетами ===
        template<typename T, typename... Args>
        T* CreateWidget(Args&&... args) {
            auto widget = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = widget.get();
            widgets_by_id_[widget->GetId()] = ptr;
            widgets_.push_back(std::move(widget));
            return ptr;
        }

        // Добавляет виджет в менеджер
        void AddWidget(std::unique_ptr<Widget> widget) {
            widgets_.push_back(std::move(widget));
        }

        //// Создает виджет по имени класса
        //Widget* CreateWidgetByClass(const std::string& widget_class,
        //    const std::string& name,
        //    const ImVec2& pos) {
        //    auto widget = WidgetFactory::CreateWithName(widget_class, name, pos);
        //    if (!widget) return nullptr;

        //    Widget* ptr = widget.get();
        //    widgets_.push_back(std::move(widget));
        //    return ptr;
        //}

        void DeleteWidget(const std::string& id);
        void DeleteWidget(int index);
        void ClearAll() { widgets_.clear(); widgets_by_id_.clear(); };
        void BringToFront(const std::string& id) {
            
            auto it = std::find_if(widgets_.begin(), widgets_.end(),
                [&](const std::unique_ptr<Widget>& w) {
                    return w->GetId() == id;
                });

            if (it != widgets_.end()) {               
                std::unique_ptr<Widget> widget_to_move = std::move(*it);
                widgets_.erase(it);
                widgets_.push_back(std::move(widget_to_move));                
                widgets_by_id_[id] = widgets_.back().get();
            }
        }

        // === Селекция ===
        void SelectWidget(const std::string& id);
        void SelectWidget(int index);
        void DeselectAll();
        Widget* GetSelectedWidget() const;

        // === Обновление и отрисовка ===
        void UpdateAll(const ImVec2& canvas_p0, const ImVec2& canvas_size);
        void RenderAll(ImDrawList* draw_list, const ImVec2& canvas_p0);
        void RenderContentAll(const ImVec2& canvas_p0);

        // === Доступ к виджетам ===
        size_t GetCount() const { return widgets_.size(); };
        Widget* GetWidget(int index) const { return widgets_[index].get(); };
        Widget* GetWidget(const std::string& id) {
            if (widgets_by_id_.count(id) > 0) {
                return widgets_by_id_.at(id); 
            }
            return nullptr;
        };

        std::vector<Widget*> GetAllWidgets() const {
            std::vector<Widget*> result;
            result.reserve(widgets_.size());

            for (const auto& widget : widgets_) {
                result.push_back(widget.get());  
            }

            return result;
        };

        // === Сериализация ===
        nlohmann::json ToJson(const wg::RuntimeWindowProperties& mw_props) const;
        void FromJson(const nlohmann::json& json);
        void SaveToFile(const std::string& filename, const wg::RuntimeWindowProperties& w_props) const;
        void LoadFromFile(const std::string& filename, wg::RuntimeWindowProperties& w_props);

    private:
        void BringToFront(Widget* widget);
    };
}