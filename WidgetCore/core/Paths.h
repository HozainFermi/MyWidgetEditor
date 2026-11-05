#pragma once
#include <filesystem>
#include <string>

namespace core {

    // Где лежат данные программы. Порядок поиска:
    //   1) рядом с exe - так работает собранная папка, унесённая на другой ПК (build_release.bat);
    //   2) папка исходников, зашитая при компиляции - так работает разработка из папки сборки.

    // Папка, где лежит exe
    std::filesystem::path ExeDir();

    // <exe>/<name>, если такая папка есть, иначе dev_fallback
    std::string DataDir(const std::string& name, const std::string& dev_fallback);

    // Папка assets (шрифты, шейдеры, модели)
    std::string AssetsDir();

    // Путь из конфига -> реальный путь:
    //  "assets/x/y"            -> <AssetsDir>/x/y
    //  "D:/old/place/assets/x" -> <AssetsDir>/x, если старого файла нет (конфиг сделан на другом ПК)
    std::string ResolveAssetPath(const std::string& path);

    // Обратно для сохранения: путь внутри AssetsDir -> "assets/...", чтобы конфиг работал на другом ПК
    std::string PortableAssetPath(const std::string& path);
}
