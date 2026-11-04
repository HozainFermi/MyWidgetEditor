#include "Config.h"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include "WidgetFactory.h"

namespace core {

    namespace {
        void SetError(std::string* error, const std::string& message) {
            if (error) {
                *error = message;
            }
        }

        bool ReadJsonFile(const std::string& path, nlohmann::json& out, std::string* error) {
            std::ifstream file(path);
            if (!file.is_open()) {
                SetError(error, "Can't open config: " + path);
                return false;
            }
            try {
                file >> out;
            }
            catch (const nlohmann::json::exception& e) {
                SetError(error, std::string("Invalid JSON in ") + path + ": " + e.what());
                return false;
            }
            return true;
        }
    }

    bool ParseConfig(const nlohmann::json& json, ConfigData& out, std::string* error) {
        out.widgets.clear();
        out.connections.clear();

        if (!json.contains("window") || !json.contains("widgets") || !json["widgets"].is_array()) {
            SetError(error, "Config must contain \"window\" and \"widgets\"");
            return false;
        }

        WindowPropertiesFromJson(json["window"], out.window);

        for (const auto& widget_json : json["widgets"]) {
            std::unique_ptr<Widget> widget = WidgetFactory::CreateFromJson(widget_json);
            if (widget) {
                out.widgets.push_back(std::move(widget));
            }
            else {
                std::cerr << "Unknown widget class skipped: " << widget_json.value("widget_class", "<none>") << std::endl;
            }
        }

        if (json.contains("connections") && json["connections"].is_array()) {
            for (const auto& c : json["connections"]) {
                AddUniqueConnection(out.connections, Connection{
                    PortRef{ c.value("from_port", ""), c.value("from_widget", "") },
                    PortRef{ c.value("to_port", ""), c.value("to_widget", "") } });
            }
        }
        return true;
    }

    bool LoadConfig(const std::string& path, ConfigData& out, std::string* error) {
        nlohmann::json json;
        return ReadJsonFile(path, json, error) && ParseConfig(json, out, error);
    }

    bool LoadWindowProperties(const std::string& path, WindowProperties& out, std::string* error) {
        nlohmann::json json;
        if (!ReadJsonFile(path, json, error)) {
            return false;
        }
        if (!json.contains("window")) {
            SetError(error, "Config has no \"window\" section");
            return false;
        }
        WindowPropertiesFromJson(json["window"], out);
        return true;
    }

    nlohmann::json ConfigToJson(const WindowProperties& window,
        const std::vector<const Widget*>& widgets,
        const std::vector<Connection>& connections) {
        nlohmann::json widgets_json = nlohmann::json::array();
        for (const Widget* widget : widgets) {
            widgets_json.push_back(widget->ToJson());
        }

        nlohmann::json connections_json = nlohmann::json::array();
        for (const auto& c : connections) {
            connections_json.push_back({
                {"from_widget", c.from.widget_id},
                {"from_port",   c.from.port},
                {"to_widget",   c.to.widget_id},
                {"to_port",     c.to.port}
            });
        }

        return {
            {"window", WindowPropertiesToJson(window)},
            {"widgets", widgets_json},
            {"connections", connections_json}
        };
    }

    bool SaveConfig(const std::string& path, const nlohmann::json& json, std::string* error) {
        std::ofstream file(path);
        if (!file.is_open()) {
            SetError(error, "Can't write config: " + path);
            return false;
        }
        file << json.dump(4);
        return true;
    }

    void ConnectPorts(const std::vector<Widget*>& widgets, const std::vector<Connection>& connections) {
        std::unordered_map<std::string, Widget*> by_id;
        for (Widget* widget : widgets) {
            widget->ClearPortCallbacks();
            by_id[widget->GetId()] = widget;
        }

        for (const auto& connection : connections) {
            auto from = by_id.find(connection.from.widget_id);
            auto to = by_id.find(connection.to.widget_id);
            if (from == by_id.end() || to == by_id.end()) {
                continue;
            }
            Widget* target = to->second;
            std::string to_port = connection.to.port;
            from->second->AddPortCallback(connection.from.port,
                [target, to_port](const std::string& from_widget_id, const std::vector<WidgetValue>& value) {
                    target->OnInput(to_port, from_widget_id, value);
                });
        }

        for (Widget* widget : widgets) {
            widget->OnPortsConnected();
        }
    }
}
