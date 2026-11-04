#pragma once
#include <string>
#include <vector>
#include "imgui.h"


// Связи между виджетами
struct PortRef {
    std::string port;
    std::string widget_id;
    //оператор сравнения для поиска в векторе
    //bool operator==(const PortRef& other) const {
    //    return port == other.port && widget_id == other.widget_id;
    //}
};

// Ключ порта для поиска в map: "<widget_id>/<port>"
inline std::string PortKey(const PortRef& ref) {
    return ref.widget_id + '/' + ref.port;
}

struct Connection {
    PortRef from;
    PortRef to;
};

// для отрисовки и взаимодействия
struct PortVisual {
    PortRef ref;
    ImVec2  pos;
    bool    is_input;
};
