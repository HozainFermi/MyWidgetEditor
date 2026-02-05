# Настройка CMake для MyWidgetEditor

## Структура проекта

Проект теперь использует CMake для сборки с общей структурой библиотек:

```
MyWidgetEditor/
├── CMakeLists.txt              # Корневой CMake файл
├── external/                   # Общие внешние библиотеки
│   └── CMakeLists.txt          # Конфигурация общих библиотек (imgui, glfw, json, httplib)
├── shared/                     # Общие компоненты (зарезервировано для будущего использования)
│   └── widgets/                # Папка для общих виджетов (если нужно)
├── MyWidgetEditor/             # Редактор виджетов
│   └── CMakeLists.txt
└── Drawer/                     # Runtime приложение
    └── CMakeLists.txt
```

## Текущая конфигурация

### Общие библиотеки (external/)

Все внешние библиотеки теперь настраиваются в `external/CMakeLists.txt`:
- **ImGui** - из `Drawer/lib/imgui` или `MyWidgetEditor/lib/imgui`
- **GLFW** - из `Drawer/lib/glfw` или `MyWidgetEditor/lib/glfw`
- **JSON (nlohmann)** - header-only из `Drawer/lib/json.hpp` или `MyWidgetEditor/lib/json.hpp`
- **httplib** - header-only из `Drawer/lib/httplib.h` или `MyWidgetEditor/lib/httplib.h`

### Виджеты

Виджеты в настоящее время остаются в своих проектах:
- `MyWidgetEditor/src/widgets/` - виджеты для редактора (namespace `wg::`)
- `Drawer/runtime_widgets/` - виджеты для runtime (namespace `rn::`)

Это связано с тем, что виджеты имеют разные интерфейсы и namespace.

## Сборка проекта

### Использование CMake Presets

```bash
# Настроить проект
cmake --preset x64-debug

# Собрать проект
cmake --build out/build/x64-debug

# Или использовать Visual Studio
# Откройте папку проекта в Visual Studio 2022+
```

### Ручная настройка CMake

```bash
# Создать директорию сборки
mkdir build
cd build

# Настроить проект
cmake ..

# Собрать
cmake --build . --config Debug
# или
cmake --build . --config Release
```

## Перемещение виджетов в общую папку (опционально)

Если вы хотите объединить виджеты в одном месте, есть несколько вариантов:

### Вариант 1: Общая библиотека виджетов с условной компиляцией

1. Создать `shared/widgets/CMakeLists.txt` для библиотеки виджетов
2. Переместить общие виджеты в `shared/widgets/`
3. Использовать условную компиляцию для поддержки обоих namespace

### Вариант 2: Общая библиотека с единым namespace

1. Объединить виджеты в `shared/widgets/` с единым namespace
2. Обновить код в обоих проектах для использования общего namespace
3. Создать адаптеры, если нужна обратная совместимость

### Вариант 3: Оставить как есть (рекомендуется)

Текущая структура работает хорошо, так как:
- Виджеты имеют разные интерфейсы (разные сигнатуры методов Render)
- Разные namespace предотвращают конфликты
- Каждый проект может развиваться независимо

## Миграция библиотек в external/ (будущее улучшение)

В будущем можно физически переместить библиотеки в `external/`:

```
external/
├── imgui/
│   └── [файлы imgui]
├── glfw/
│   └── [файлы glfw]
├── json.hpp
└── httplib.h
```

Тогда нужно будет обновить пути в `external/CMakeLists.txt`.

## Примечания

- GLFW DLL автоматически копируется в директорию выходных файлов при сборке
- Конфигурационные файлы JSON копируются в выходную директорию для MyWidgetEditor
- Все проекты используют C++17 стандарт
