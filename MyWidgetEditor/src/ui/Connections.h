#pragma once
#include <imgui.h>
#include <core/Ports.h>

using core::Connection;
using core::PortKey;
using core::PortRef;
using core::SamePort;

// Порт на канвасе редактора: для отрисовки и перетаскивания связей
struct PortVisual {
    PortRef ref;
    ImVec2  pos;
    bool    is_input;
};
