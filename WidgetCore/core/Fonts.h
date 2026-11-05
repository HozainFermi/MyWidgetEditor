#pragma once

namespace core {
    // Основной шрифт: встроенный в ImGui ProggyClean (латиница как в оригинале) +
    // кириллица из Terminus, которой в ProggyClean нет. ImGui сам берёт недостающие буквы
    // из подмешанного шрифта. Вызывать до первого кадра, после ImGui::CreateContext()
    void AddMainFont();
}
