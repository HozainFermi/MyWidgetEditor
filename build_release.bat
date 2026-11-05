@echo off
rem Собирает Release и складывает всё нужное для запуска на другом ПК в dist\MyWidgetEditor.
rem Нужны: Visual Studio с C++ (ставит MSVC, CMake и Ninja) и OpenSSL в C:\Program Files\OpenSSL-Win64.
rem Папку dist\MyWidgetEditor можно скопировать на любой Windows x64 и запускать MyWidgetEditor.exe.
setlocal
chcp 65001 >nul

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VS=%%i"
if not defined VS (
    echo Visual Studio with C++ tools not found
    exit /b 1
)

call "%VS%\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1
set "PATH=%VS%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%VS%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"

cd /d "%~dp0"
cmake -S . -B out\build\x64-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe || exit /b 1
cmake --build out\build\x64-release || exit /b 1

if exist dist\MyWidgetEditor rmdir /s /q dist\MyWidgetEditor
cmake --install out\build\x64-release --prefix dist\MyWidgetEditor || exit /b 1

echo.
echo Done: %~dp0dist\MyWidgetEditor
