#include "Fonts.h"
#include <string>
#include <imgui.h>
#include "Paths.h"

namespace core {

    void AddMainFont() {
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontDefault();  // ProggyClean 13px

        // Только недостающие глифы (кириллица) - из Terminus, без сглаживания по субпикселям
        ImFontConfig cyrillic;
        cyrillic.MergeMode = true;
        cyrillic.OversampleH = 1;
        cyrillic.OversampleV = 1;
        cyrillic.PixelSnapH = true;
        const std::string path = AssetsDir() + "/fonts/TerminusTTF.ttf";
        io.Fonts->AddFontFromFileTTF(path.c_str(), 13.0f, &cyrillic);
    }
}
