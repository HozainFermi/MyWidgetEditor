#pragma once
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <json.hpp>
#include "Process.h"

// Запущенные виджеты. Список хранится в файле, поэтому виджеты переживают закрытие редактора,
// а при следующем запуске редактор их подхватывает и может закрыть.
class ProcessList {
public:
    // Читает файл и подхватывает процессы, которые всё ещё работают
    void Load() {
        items_.clear();
        std::ifstream file(FilePath());
        if (file.is_open()) {
            try {
                nlohmann::json json;
                file >> json;
                for (const auto& entry : json.value("processes", nlohmann::json::array())) {
                    Process process;
                    if (Process::Attach(entry.value("pid", 0u), entry.value("created", uint64_t(0)),
                        entry.value("name", ""), process)) {
                        items_.push_back(std::move(process));
                    }
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Can't read " << FilePath() << ": " << e.what() << std::endl;
            }
        }
        Save();  // в файле остаются только живые
    }

    void Add(Process process) {
        items_.push_back(std::move(process));
        Save();
    }

    void Kill(size_t index) {
        if (index >= items_.size()) return;
        items_[index].Kill();
        items_.erase(items_.begin() + index);
        Save();
    }

    void KillAll() {
        for (auto& process : items_) {
            process.Kill();
        }
        items_.clear();
        Save();
    }

    // Убирает виджеты, которые закрылись сами
    void RemoveExited() {
        size_t before = items_.size();
        std::erase_if(items_, [](const Process& p) { return !p.IsRunning(); });
        if (items_.size() != before) {
            Save();
        }
    }

    size_t size() const { return items_.size(); }
    bool empty() const { return items_.empty(); }
    const Process& operator[](size_t index) const { return items_[index]; }

    static std::string FilePath() {
        const char* base = std::getenv("LOCALAPPDATA");
        std::filesystem::path dir = base ? std::filesystem::path(base) : std::filesystem::temp_directory_path();
        return (dir / "MyWidgetEditor" / "processes.json").string();
    }

private:
    void Save() const {
        nlohmann::json list = nlohmann::json::array();
        for (const auto& process : items_) {
            list.push_back({ {"pid", process.GetPid()}, {"created", process.GetCreationTime()}, {"name", process.GetName()} });
        }
        std::filesystem::path path = FilePath();
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path(), ec);
        std::ofstream file(path);
        if (file.is_open()) {
            file << nlohmann::json{ {"processes", list} }.dump(2);
        }
    }

    std::vector<Process> items_;
};
