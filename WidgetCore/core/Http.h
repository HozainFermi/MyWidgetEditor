#pragma once
#include <map>
#include <string>

namespace core {

    // Синхронный GET (http/https). Бросает исключение при ошибке сети или статусе не 2xx.
    // Блокирует поток - вызывать из фонового потока, не из Update()/Render()
    std::string HttpGet(const std::string& url, const std::map<std::string, std::string>& headers = {});

    // Кодирует строку (UTF-8) для подстановки в URL: "Нижний Новгород" -> "%D0%9D..."
    std::string UrlEncode(const std::string& text);
}
