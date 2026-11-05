#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <json.hpp>
#include "Widget.h"

namespace core {

    enum class WidgetCategory {
        Layout,  // виджеты окна (текст, таблица, график...)
        Logic    // узлы логики (Filter, Map...) - только на вкладке Logic
    };

    // Создание виджетов по имени класса ("TextWidget", "TableWidget"...).
    // Регистрация явная - через RegisterAllWidgets(), без статических переменных:
    // линкер MSVC выкидывает из статической библиотеки файлы, на которые никто не ссылается.

    class WidgetFactory {
    public:
        using Creator = std::function<std::unique_ptr<Widget>()>;

        template<typename T>
        static void Register(const std::string& class_name, WidgetCategory category = WidgetCategory::Layout) {
            Creators()[class_name] = []() -> std::unique_ptr<Widget> { return std::make_unique<T>(); };
            Categories()[class_name] = category;
        }

        // nullptr, если класс не зарегистрирован
        static std::unique_ptr<Widget> Create(const std::string& class_name);
        // Создаёт по полю "widget_class" и загружает остальные поля
        static std::unique_ptr<Widget> CreateFromJson(const nlohmann::json& json);

        static std::vector<std::string> GetRegisteredTypes();
        static std::vector<std::string> GetRegisteredTypes(WidgetCategory category);
        static bool IsTypeRegistered(const std::string& class_name);

    private:
        static std::map<std::string, Creator>& Creators();
        static std::map<std::string, WidgetCategory>& Categories();
    };
}
