#pragma once
#include <memory>
#include <string>
#include <vector>
#include <json.hpp>
#include "Ports.h"
#include "Widget.h"
#include "WindowProperties.h"

namespace core {

    // Содержимое файла конфига: окно, виджеты и связи между ними
    struct ConfigData {
        WindowProperties window;
        std::vector<std::unique_ptr<Widget>> widgets;
        std::vector<Connection> connections;
    };

    // Разбирает конфиг. Виджеты создают OpenGL-объекты, поэтому нужен готовый GL-контекст.
    // Незнакомые классы виджетов пропускаются, дубли связей отбрасываются.
    bool ParseConfig(const nlohmann::json& json, ConfigData& out, std::string* error = nullptr);
    bool LoadConfig(const std::string& path, ConfigData& out, std::string* error = nullptr);

    // Только свойства окна - Drawer читает их до создания окна и GL-контекста
    bool LoadWindowProperties(const std::string& path, WindowProperties& out, std::string* error = nullptr);

    nlohmann::json ConfigToJson(const WindowProperties& window,
        const std::vector<const Widget*>& widgets,
        const std::vector<Connection>& connections);
    bool SaveConfig(const std::string& path, const nlohmann::json& json, std::string* error = nullptr);

    // Подключает Emit выходов ко входам по списку связей (старые подключения сбрасываются)
    void ConnectPorts(const std::vector<Widget*>& widgets, const std::vector<Connection>& connections);
}
