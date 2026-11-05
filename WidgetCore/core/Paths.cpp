#include "Paths.h"
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace core {

    namespace {
        std::string Normalize(std::string path) {
            std::replace(path.begin(), path.end(), '\\', '/');
            return path;
        }

        std::string Lower(std::string text) {
            std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return (char)std::tolower(c); });
            return text;
        }
    }

    std::filesystem::path ExeDir() {
#ifdef _WIN32
        wchar_t buffer[MAX_PATH];
        DWORD size = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        if (size > 0 && size < MAX_PATH) {
            return std::filesystem::path(buffer).parent_path();
        }
#endif
        return std::filesystem::current_path();
    }

    std::string DataDir(const std::string& name, const std::string& dev_fallback) {
        std::error_code ec;
        const std::filesystem::path near_exe = ExeDir() / name;
        if (std::filesystem::is_directory(near_exe, ec)) {
            return Normalize(near_exe.string());
        }
        return Normalize(dev_fallback);
    }

    std::string AssetsDir() {
        static const std::string dir = DataDir("assets", ASSETS_SOURCE_DIR);
        return dir;
    }

    std::string ResolveAssetPath(const std::string& raw) {
        const std::string path = Normalize(raw);
        if (path.rfind("assets/", 0) == 0) {
            return AssetsDir() + "/" + path.substr(7);
        }
        // Абсолютный путь с другого ПК: берём часть после "/assets/"
        std::error_code ec;
        if (!path.empty() && !std::filesystem::exists(path, ec)) {
            const size_t pos = Lower(path).rfind("/assets/");
            if (pos != std::string::npos) {
                return AssetsDir() + "/" + path.substr(pos + 8);
            }
        }
        return path;
    }

    std::string PortableAssetPath(const std::string& raw) {
        const std::string path = Normalize(raw);
        const std::string assets = AssetsDir() + "/";
        if (Lower(path).rfind(Lower(assets), 0) == 0) {
            return "assets/" + path.substr(assets.size());
        }
        return path;
    }
}
