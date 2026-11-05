#pragma once
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "Ports.h"

namespace core {

    class Widget;

    // Сообщение с выходного порта виджета
    struct WidgetMessage {
        std::string from_widget_id;
        std::string from_port;
        std::vector<WidgetValue> value;
    };

    // Шина событий между виджетами.
    // Post() можно вызывать из любого потока (например, из потока загрузки или MQTT-клиента).
    // Доставка (Widget::OnInput) всегда на главном потоке в Dispatch(), который менеджер зовёт раз за кадр.
    class EventBus {
    public:
        // Любой поток
        void Post(WidgetMessage message);

        // Главный поток. Кому доставлять: виджеты и связи. Вызывать после любых изменений связей/виджетов.
        // После обновления маршрутов каждый виджет получает OnPortsConnected()
        void SetRoutes(const std::vector<Widget*>& widgets, const std::vector<Connection>& connections);

        // Главный поток. Доставляет накопленные сообщения. То, что виджеты отправят внутри OnInput
        // (например, цепочка узлов Filter -> Map -> график), доставляется в этом же кадре,
        // но не больше kMaxRounds волн - так зацикленная схема не повесит программу
        void Dispatch();

        // Последние данные, прошедшие через выход (для инспектора проводов). nullptr - ещё ничего не было
        const std::vector<WidgetValue>* GetLastValue(const PortRef& output) const;

        static constexpr int kMaxRounds = 16;

    private:
        std::mutex queue_mutex_;
        std::vector<WidgetMessage> queue_;  // под queue_mutex_

        std::unordered_map<std::string, Widget*> widgets_;
        std::vector<Connection> connections_;
        std::unordered_map<std::string, std::vector<WidgetValue>> last_values_;  // PortKey(выход) -> данные
    };
}
