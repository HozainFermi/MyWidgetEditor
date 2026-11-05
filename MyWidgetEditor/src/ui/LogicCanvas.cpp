// Вкладка Logic: схема данных. Виджеты и узлы логики - блоки с портами, связи - провода.
// Положение блоков здесь своё (graph_position) и на окно виджета не влияет.
#include "EditorMainWindowLayout.h"
#include <algorithm>
#include <unordered_map>
#include <imgui_internal.h>
#include <core/WidgetFactory.h>

namespace {
    const float kNodeWidth = 180.0f;
    const float kHeaderHeight = 24.0f;
    const float kRowHeight = 20.0f;
    const float kPortRadius = 5.0f;

    ImVec2 NodeSize(const core::Widget& widget) {
        const size_t rows = std::max(widget.GetInputPorts().size(), widget.GetOutputPorts().size());
        return ImVec2(kNodeWidth, kHeaderHeight + std::max<size_t>(rows, 1) * kRowHeight + 6.0f);
    }

    // Короткий текст о данных на проводе для инспектора
    std::string DescribeValue(const std::vector<core::WidgetValue>* value) {
        if (!value) {
            return "No data has passed yet";
        }
        std::string text = std::to_string(value->size()) + (value->size() == 1 ? " row" : " rows");
        const size_t shown = std::min<size_t>(value->size(), 3);
        for (size_t i = 0; i < shown; ++i) {
            std::string row = (*value)[i].dump();
            if (row.size() > 90) row = row.substr(0, 87) + "...";
            text += "\n" + row;
        }
        if (value->size() > shown) {
            text += "\n...";
        }
        return text;
    }

    void DrawWire(ImDrawList* dl, ImVec2 from, ImVec2 to, ImU32 color, float thickness) {
        const float dx = std::max(40.0f, std::abs(to.x - from.x) * 0.5f);
        dl->AddBezierCubic(from, ImVec2(from.x + dx, from.y), ImVec2(to.x - dx, to.y), to, color, thickness);
    }

    float WireDistance(ImVec2 from, ImVec2 to, ImVec2 point) {
        const float dx = std::max(40.0f, std::abs(to.x - from.x) * 0.5f);
        ImVec2 closest = ImBezierCubicClosestPointCasteljau(from, ImVec2(from.x + dx, from.y), ImVec2(to.x - dx, to.y), to, point, 1.0f);
        return ImLengthSqr(ImVec2(closest.x - point.x, closest.y - point.y));
    }
}

void Editor::RenderLogicCanvas() {
    // Данные должны течь и на этой вкладке (перетаскивание раскладки здесь не нужно)
    widget_manager_.UpdateWidgets();

    ImGui::TextDisabled("Drag nodes from the left panel. Drag from a port to connect, right-click a wire to delete it,\nhover a wire to see its data. Middle mouse - pan.");

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    ImGui::InvisibleButton("LogicCanvas", avail, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle);
    const ImVec2 p0 = ImGui::GetItemRectMin();
    const ImVec2 p1 = ImGui::GetItemRectMax();
    const bool canvas_hovered = ImGui::IsItemHovered();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImGuiIO& io = ImGui::GetIO();

    // Панорамирование средней кнопкой
    if (canvas_hovered && ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
        logic_pan_.x += io.MouseDelta.x;
        logic_pan_.y += io.MouseDelta.y;
    }
    const ImVec2 origin(p0.x + logic_pan_.x, p0.y + logic_pan_.y);

    dl->PushClipRect(p0, p1, true);
    dl->AddRectFilled(p0, p1, IM_COL32(22, 24, 30, 255));
    for (float x = fmodf(logic_pan_.x, 32.0f); x < avail.x; x += 32.0f)
        dl->AddLine(ImVec2(p0.x + x, p0.y), ImVec2(p0.x + x, p1.y), IM_COL32(255, 255, 255, 12));
    for (float y = fmodf(logic_pan_.y, 32.0f); y < avail.y; y += 32.0f)
        dl->AddLine(ImVec2(p0.x, p0.y + y), ImVec2(p1.x, p0.y + y), IM_COL32(255, 255, 255, 12));

    // Новый узел перетащили из левой панели
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LOGIC_NODE")) {
            const ImVec2 at(io.MousePos.x - origin.x, io.MousePos.y - origin.y);
            CreateWidgetFromTemplate((const char*)payload->Data, at);
        }
        ImGui::EndDragDropTarget();
    }

    // У кого ещё нет места на схеме - раскладываем по потоку данных:
    // колонка = длина самой длинной цепочки до блока (источники слева), внутри колонки - сверху вниз
    std::vector<core::Widget*> widgets = widget_manager_.GetAllWidgets();
    std::unordered_map<std::string, int> depth;
    for (core::Widget* w : widgets) depth[w->GetId()] = 0;
    for (size_t pass = 0; pass < widgets.size(); ++pass) {  // не больше N проходов - защита от циклов
        bool changed = false;
        for (const auto& c : widget_manager_.GetConnections()) {
            if (depth.count(c.from.widget_id) && depth.count(c.to.widget_id) &&
                depth[c.to.widget_id] < depth[c.from.widget_id] + 1) {
                depth[c.to.widget_id] = depth[c.from.widget_id] + 1;
                changed = true;
            }
        }
        if (!changed) break;
    }
    std::unordered_map<int, int> rows_in_column;
    for (core::Widget* w : widgets) {
        if (!w->HasGraphPosition()) {
            const int column = depth[w->GetId()];
            const int row = rows_in_column[column]++;
            w->SetGraphPosition(ImVec2(30.0f + column * 230.0f, 30.0f + row * 110.0f));
        }
    }

    // ===== Блоки и порты =====
    port_visuals_.clear();
    std::string hovered_node;
    for (core::Widget* w : widgets) {
        const ImVec2 size = NodeSize(*w);
        const ImVec2 min(origin.x + w->GetGraphPosition().x, origin.y + w->GetGraphPosition().y);
        const ImVec2 max(min.x + size.x, min.y + size.y);
        const bool selected = widget_manager_.IsSelected(w);
        const bool logic = w->IsLogicNode();

        dl->AddRectFilled(min, max, IM_COL32(38, 42, 54, 245), 6.0f);
        dl->AddRectFilled(min, ImVec2(max.x, min.y + kHeaderHeight),
            logic ? IM_COL32(110, 70, 150, 255) : IM_COL32(50, 90, 150, 255), 6.0f, ImDrawFlags_RoundCornersTop);
        dl->AddRect(min, max, selected ? IM_COL32(230, 210, 120, 255) : IM_COL32(80, 85, 100, 255), 6.0f, 0, selected ? 2.0f : 1.0f);
        dl->AddText(ImVec2(min.x + 8, min.y + 4), IM_COL32(255, 255, 255, 255), w->GetName().c_str());

        const auto inputs = w->GetInputPorts();
        const auto outputs = w->GetOutputPorts();
        for (size_t i = 0; i < inputs.size(); ++i) {
            const ImVec2 p(min.x, min.y + kHeaderHeight + i * kRowHeight + kRowHeight * 0.5f);
            port_visuals_.push_back(PortVisual{ PortRef{ inputs[i].name, w->GetId() }, p, true });
            dl->AddCircleFilled(p, kPortRadius, IM_COL32(180, 220, 180, 255));
            dl->AddText(ImVec2(p.x + 10, p.y - 8), IM_COL32(220, 220, 230, 255), inputs[i].label.c_str());
        }
        for (size_t i = 0; i < outputs.size(); ++i) {
            const ImVec2 p(max.x, min.y + kHeaderHeight + i * kRowHeight + kRowHeight * 0.5f);
            port_visuals_.push_back(PortVisual{ PortRef{ outputs[i].name, w->GetId() }, p, false });
            dl->AddCircleFilled(p, kPortRadius, IM_COL32(220, 180, 180, 255));
            const float tw = ImGui::CalcTextSize(outputs[i].label.c_str()).x;
            dl->AddText(ImVec2(p.x - 10 - tw, p.y - 8), IM_COL32(220, 220, 230, 255), outputs[i].label.c_str());
        }
        if (inputs.empty() && outputs.empty()) {
            dl->AddText(ImVec2(min.x + 8, min.y + kHeaderHeight + 2), IM_COL32(150, 150, 160, 255), "no ports");
        }

        if (canvas_hovered && ImRect(min, max).Contains(io.MousePos)) {
            hovered_node = w->GetId();  // последний - верхний
        }
    }

    // ===== Мышь: порт -> новая связь, блок -> выделить и тащить =====
    auto port_under_mouse = [&]() -> const PortVisual* {
        for (const auto& v : port_visuals_) {
            if (ImLengthSqr(ImVec2(io.MousePos.x - v.pos.x, io.MousePos.y - v.pos.y)) <= (kPortRadius + 3) * (kPortRadius + 3)) {
                return &v;
            }
        }
        return nullptr;
    };

    if (canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        if (const PortVisual* port = port_under_mouse()) {
            is_dragging_connection_ = true;
            drag_from_port_ = port->ref;
        }
        else if (!hovered_node.empty()) {
            widget_manager_.Select(hovered_node);
            dragging_node_id_ = hovered_node;
        }
        else {
            widget_manager_.DeselectAll();
        }
    }
    if (!dragging_node_id_.empty() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        if (core::Widget* w = widget_manager_.GetWidget(dragging_node_id_)) {
            const ImVec2 pos = w->GetGraphPosition();
            w->SetGraphPosition(ImVec2(pos.x + io.MouseDelta.x, pos.y + io.MouseDelta.y));
        }
    }

    // ===== Провода, инспектор, удаление =====
    std::unordered_map<std::string, ImVec2> port_positions;
    for (const auto& v : port_visuals_) {
        port_positions[PortKey(v.ref)] = v.pos;
    }
    int hovered_wire = -1;
    const auto& connections = widget_manager_.GetConnections();
    for (int i = 0; i < (int)connections.size(); ++i) {
        auto from = port_positions.find(PortKey(connections[i].from));
        auto to = port_positions.find(PortKey(connections[i].to));
        if (from == port_positions.end() || to == port_positions.end()) continue;
        if (hovered_wire < 0 && canvas_hovered && !is_dragging_connection_ && dragging_node_id_.empty() &&
            WireDistance(from->second, to->second, io.MousePos) < 36.0f) {
            hovered_wire = i;
        }
        DrawWire(dl, from->second, to->second, i == hovered_wire ? IM_COL32(255, 220, 120, 255) : IM_COL32(150, 150, 255, 255), i == hovered_wire ? 3.5f : 2.0f);
    }
    if (hovered_wire >= 0) {
        const core::Connection& c = connections[hovered_wire];
        ImGui::SetTooltip("%s.%s -> %s.%s\n%s", c.from.widget_id.c_str(), c.from.port.c_str(), c.to.widget_id.c_str(), c.to.port.c_str(),
            DescribeValue(widget_manager_.GetBus().GetLastValue(c.from)).c_str());
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            widget_manager_.RemoveConnection(hovered_wire);
        }
    }

    if (is_dragging_connection_) {
        auto from = port_positions.find(PortKey(drag_from_port_));
        if (from != port_positions.end()) {
            DrawWire(dl, from->second, io.MousePos, IM_COL32(200, 200, 100, 255), 2.0f);
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            const PortVisual* target = port_under_mouse();
            const PortVisual* source = nullptr;
            for (const auto& v : port_visuals_) {
                if (SamePort(v.ref, drag_from_port_)) source = &v;
            }
            // Только выход -> вход разных блоков; потянули от входа - разворачиваем
            if (source && target && source->is_input != target->is_input && source->ref.widget_id != target->ref.widget_id) {
                widget_manager_.AddConnection(source->is_input ? Connection{ target->ref, source->ref } : Connection{ source->ref, target->ref });
            }
            is_dragging_connection_ = false;
        }
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        dragging_node_id_.clear();
    }

    // Порт под курсором - подсказка с именем
    if (const PortVisual* port = canvas_hovered ? port_under_mouse() : nullptr) {
        if (hovered_wire < 0) {
            ImGui::SetTooltip("%s (%s)", port->ref.port.c_str(), port->is_input ? "input" : "output");
        }
    }

    dl->PopClipRect();
}
