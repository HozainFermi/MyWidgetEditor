#pragma once
#include <string>
#include "imgui.h"

struct Point {
    float x;
    float y;

    Point(float x_pos = 0, float y_pos = 0) : x(x_pos), y(y_pos) {}
};

struct Widget {
    
    ImVec2 p_min;
    ImVec2 p_max;
    ImVec4 color;

    std::string Name;
    std::string Type;
    std::string Info;
    char EditBuffer[4096]; // ����� ��� ��������������

    Widget(
        ImVec2 left_bot_p,
        ImVec2 right_up_p,
        ImVec4 col,
        std::string name = "",
        std::string type = "",
        std::string info = "")

        : p_min(left_bot_p), p_max(right_up_p), color(col), Name(name), Type(type), Info(info)
    {
        // �������������� �����
        strncpy_s(EditBuffer, info.c_str(), sizeof(EditBuffer) - 1);
        EditBuffer[sizeof(EditBuffer) - 1] = '\0';
    }

    Widget(
        ImVec2 left_bot_p,
        ImVec2 right_up_p,       
        std::string name = "",
        std::string type = "",
        std::string info = "")

        : p_min(left_bot_p), p_max(right_up_p), Name(name), Type(type), Info(info)
    {
        // �������������� �����
        strncpy_s(EditBuffer, info.c_str(), sizeof(EditBuffer) - 1);
        EditBuffer[sizeof(EditBuffer) - 1] = '\0';
    }
};