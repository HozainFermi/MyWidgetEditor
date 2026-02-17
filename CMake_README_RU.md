# Настройка CMake для проекта MyWidgetEditor

## Что было сделано

✅ Создана структура для общих библиотек в `external/`
✅ Настроены CMakeLists.txt для обоих проектов (MyWidgetEditor и Drawer)
✅ Библиотеки (imgui, glfw, json, httplib) теперь используются из одного места
✅ GLFW DLL автоматически копируется при сборке

## Структура

```
MyWidgetEditor/
├── CMakeLists.txt              # Корневой файл
├── external/
│   └── CMakeLists.txt          # Общие библиотеки
├── MyWidgetEditor/
│   └── CMakeLists.txt          # Редактор
└── Drawer/
    └── CMakeLists.txt          # Runtime приложение
```

## Как собрать

### Через CMake Presets (рекомендуется)

```powershell
# Настроить проект
cmake --preset x64-debug

# Собрать
cmake --build out/build/x64-debug
```

### Через Visual Studio

1. Откройте папку проекта в Visual Studio 2022+
2. Выберите preset (x64-debug или x64-release)
3. Соберите проект

## О виджетах

Виджеты сейчас остаются в своих проектах:
- `MyWidgetEditor/src/widgets/` - для редактора (namespace `wg::`)
- `Drawer/runtime_widgets/` - для runtime (namespace `rn::`)

Это связано с разными интерфейсами и namespace. Если нужно объединить их, см. `CMake_SETUP.md`.

## Библиотеки

Все библиотеки автоматически находятся из:
- `Drawer/lib/` или `MyWidgetEditor/lib/`

В будущем можно физически переместить их в `external/` для полной централизации.
