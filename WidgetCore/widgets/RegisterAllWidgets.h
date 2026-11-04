#pragma once

namespace core {
    // Регистрирует все виджеты в WidgetFactory. Вызвать один раз при старте (повторный вызов безопасен).
    // Новый виджет = одна строка в RegisterAllWidgets.cpp
    void RegisterAllWidgets();
}
