#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <json.hpp>
#include "Widget.h"

namespace core {

    // Создание виджетов по имени класса ("TextWidget", "TableWidget"...).
    // Регистрация явная - через RegisterAllWidgets(), без статических переменных:
    // линкер MSVC выкидывает из статической библиотеки файлы, на которые никто не ссылается.
    class WidgetFactory {
    public:
        using Creator = std::function<std::unique_ptr<Widget>()>;

        template<typename T>
        static void Register(const std::string& class_name) {
            Creators()[class_name] = []() -> std::unique_ptr<Widget> { return std::make_unique<T>(); };
        }

        // nullptr, если класс не зарегистрирован
        static std::unique_ptr<Widget> Create(const std::string& class_name);
        // Создаёт по полю "widget_class" и загружает остальные поля
        static std::unique_ptr<Widget> CreateFromJson(const nlohmann::json& json);

        static std::vector<std::string> GetRegisteredTypes();
        static bool IsTypeRegistered(const std::string& class_name);

    private:
        static std::map<std::string, Creator>& Creators();
    };
}
