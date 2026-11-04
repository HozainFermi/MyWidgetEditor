#pragma once
#include <string>
#include <vector>
#include <json.hpp>

namespace core {

    // Данные между виджетами передаются как JSON (универсальный контейнер)
    using WidgetValue = nlohmann::json;

    struct PortDesc {
        std::string name;   // внутреннее имя порта
        std::string label;  // отображаемое имя
        std::string type;   // произвольный строковый тип ("event", "table", "number"...)
        bool is_input = true;
    };

    struct PortRef {
        std::string port;
        std::string widget_id;
    };

    // Связь всегда идёт от выхода (from) ко входу (to)
    struct Connection {
        PortRef from;
        PortRef to;
    };

    inline bool SamePort(const PortRef& a, const PortRef& b) {
        return a.port == b.port && a.widget_id == b.widget_id;
    }

    inline bool SameConnection(const Connection& a, const Connection& b) {
        return SamePort(a.from, b.from) && SamePort(a.to, b.to);
    }

    // Ключ порта для поиска в map: "<widget_id>/<port>"
    inline std::string PortKey(const PortRef& ref) {
        return ref.widget_id + '/' + ref.port;
    }

    // Добавляет связь, если такой ещё нет. Возвращает false для дубликата
    inline bool AddUniqueConnection(std::vector<Connection>& connections, const Connection& connection) {
        for (const auto& existing : connections) {
            if (SameConnection(existing, connection)) {
                return false;
            }
        }
        connections.push_back(connection);
        return true;
    }
}
