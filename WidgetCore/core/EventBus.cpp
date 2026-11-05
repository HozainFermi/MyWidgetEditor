#include "EventBus.h"
#include "Widget.h"

namespace core {

    void EventBus::Post(WidgetMessage message) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        queue_.push_back(std::move(message));
    }

    void EventBus::SetRoutes(const std::vector<Widget*>& widgets, const std::vector<Connection>& connections) {
        widgets_.clear();
        for (Widget* widget : widgets) {
            widgets_[widget->GetId()] = widget;
        }
        connections_ = connections;

        for (Widget* widget : widgets) {
            widget->OnPortsConnected();
        }
    }

    void EventBus::Dispatch() {
        for (int round = 0; round < kMaxRounds; ++round) {
            std::vector<WidgetMessage> messages;
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                messages.swap(queue_);
            }
            if (messages.empty()) {
                return;
            }

            for (const auto& message : messages) {
                last_values_[PortKey(PortRef{ message.from_port, message.from_widget_id })] = message.value;

                for (const auto& connection : connections_) {
                    if (connection.from.widget_id != message.from_widget_id || connection.from.port != message.from_port) {
                        continue;
                    }
                    // Виджет могли удалить, пока сообщение лежало в очереди - тогда его просто нет в карте
                    auto target = widgets_.find(connection.to.widget_id);
                    if (target != widgets_.end()) {
                        target->second->OnInput(connection.to.port, message.from_widget_id, message.value);
                    }
                }
            }
        }
        // Остаток (если схема зациклена) доставится в следующем кадре
    }

    const std::vector<WidgetValue>* EventBus::GetLastValue(const PortRef& output) const {
        auto it = last_values_.find(PortKey(output));
        return it != last_values_.end() ? &it->second : nullptr;
    }
}
