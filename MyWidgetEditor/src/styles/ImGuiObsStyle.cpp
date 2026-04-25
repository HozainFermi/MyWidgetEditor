#include "ImGuiObsStyle.h"

#include "imgui.h"

void ApplyObsLikeImGuiStyle(float main_scale)
{
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowPadding = ImVec2(10.0f, 8.0f);
    style.FramePadding = ImVec2(8.0f, 5.0f);
    style.CellPadding = ImVec2(8.0f, 6.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 5.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 13.0f;
    style.GrabMinSize = 11.0f;

    style.WindowRounding = 6.0f;
    style.ChildRounding = 5.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 4.0f;

    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.TabBorderSize = 1.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.92f, 0.94f, 0.96f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.53f, 0.57f, 0.63f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.13f, 0.16f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.15f, 0.18f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.24f, 0.27f, 0.31f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    colors[ImGuiCol_FrameBg] = ImVec4(0.17f, 0.19f, 0.23f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.25f, 0.31f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.19f, 0.30f, 0.51f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.19f, 0.23f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);

    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.11f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.26f, 0.30f, 0.37f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.31f, 0.36f, 0.44f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.20f, 0.46f, 0.78f, 1.00f);

    colors[ImGuiCol_CheckMark] = ImVec4(0.20f, 0.60f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.20f, 0.60f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.34f, 0.71f, 1.00f, 1.00f);

    colors[ImGuiCol_Button] = ImVec4(0.18f, 0.21f, 0.26f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.23f, 0.28f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.48f, 0.82f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.22f, 0.27f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.29f, 0.36f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.48f, 0.82f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.24f, 0.27f, 0.31f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.20f, 0.48f, 0.82f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.20f, 0.48f, 0.82f, 1.00f);

    colors[ImGuiCol_ResizeGrip] = ImVec4(0.20f, 0.48f, 0.82f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.20f, 0.60f, 0.98f, 0.78f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.34f, 0.71f, 1.00f, 1.00f);

    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.28f, 0.40f, 1.00f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.20f, 0.48f, 0.82f, 1.00f);
    colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.34f, 0.71f, 1.00f, 1.00f);
    colors[ImGuiCol_TabDimmed] = ImVec4(0.13f, 0.14f, 0.18f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.17f, 0.21f, 0.27f, 1.00f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.20f, 0.48f, 0.82f, 1.00f);

    //colors[ImGuiCol_DockingPreview] = ImVec4(0.20f, 0.60f, 0.98f, 0.70f);
    //colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.09f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.65f, 0.70f, 0.76f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.20f, 0.60f, 0.98f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.20f, 0.60f, 0.98f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.34f, 0.71f, 1.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.16f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.25f, 0.28f, 0.34f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.19f, 0.21f, 0.26f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.16f, 0.18f, 0.22f, 0.35f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.48f, 0.82f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.34f, 0.71f, 1.00f, 0.90f);
    colors[ImGuiCol_NavCursor] = ImVec4(0.34f, 0.71f, 1.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.34f, 0.71f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.05f, 0.06f, 0.08f, 0.60f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.05f, 0.06f, 0.08f, 0.70f);

    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;
}
