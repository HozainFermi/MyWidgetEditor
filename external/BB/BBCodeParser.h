#pragma once
#include <vector>
#include <string>
#include <stack>
#include <stdexcept>
#include <cstdint>


// Битовые флаги для текста.
// Стили (b, i) можно комбинировать как угодно.
// Эффекты (wave, rainbow, shake) взаимоисключающие: внутри одного эффекта нельзя открыть другой.
enum TextEffects_ {
    TextEffect_None = 0,
    TextEffect_Bold = 1 << 0,
    TextEffect_Italic = 1 << 1,
    TextEffect_Wave = 1 << 2,
    TextEffect_Rainbow = 1 << 3,
    TextEffect_Shake = 1 << 4,

    TextStyle_Mask = TextEffect_Bold | TextEffect_Italic,
    TextEffect_Mask = TextEffect_Wave | TextEffect_Rainbow | TextEffect_Shake
};

struct TextSegment {
    std::string content; // Текст без тегов
    uint32_t flags;      // Активные стили и эффекты
};


class BBCodeParser {
public:
    // Бросает std::invalid_argument при ошибке разметки.
    // Незнакомые теги ([foo]) и одиночная '[' остаются обычным текстом.
    static std::vector<TextSegment> Parse(const std::string& input) {
        std::vector<TextSegment> result;
        std::stack<std::string> active_tags;
        uint32_t current_flags = TextEffect_None;
        std::string plain_text;

        auto flush = [&]() {
            if (!plain_text.empty()) {
                result.push_back({ plain_text, current_flags });
                plain_text.clear();
            }
        };

        for (size_t i = 0; i < input.length(); ++i) {
            if (input[i] != '[') {
                plain_text += input[i];
                continue;
            }

            size_t close = input.find(']', i + 1);
            if (close == std::string::npos) {
                plain_text += input.substr(i);
                break;
            }

            std::string tag = input.substr(i + 1, close - i - 1);
            bool is_closing = !tag.empty() && tag[0] == '/';
            std::string name = is_closing ? tag.substr(1) : tag;
            uint32_t flag = GetFlagForTag(name);

            if (flag == 0) {
                // Незнакомый тег - оставляем как есть
                plain_text += input.substr(i, close - i + 1);
                i = close;
                continue;
            }

            flush();

            if (is_closing) {
                if (active_tags.empty() || active_tags.top() != name) {
                    throw std::invalid_argument("Tags are overlapping or unmatched: [/" + name + "]");
                }
                current_flags &= ~flag;
                active_tags.pop();
            }
            else {
                if (current_flags & flag) {
                    throw std::invalid_argument("Tag is already open: [" + name + "]");
                }
                if ((flag & TextEffect_Mask) && (current_flags & TextEffect_Mask)) {
                    throw std::invalid_argument("Effects can't be nested: [" + name + "]");
                }
                active_tags.push(name);
                current_flags |= flag;
            }
            i = close;
        }

        flush();
        return result;
    }

private:
    static uint32_t GetFlagForTag(const std::string& tag) {
        if (tag == "b") return TextEffect_Bold;
        if (tag == "i") return TextEffect_Italic;
        if (tag == "wave") return TextEffect_Wave;
        if (tag == "rainbow") return TextEffect_Rainbow;
        if (tag == "shake") return TextEffect_Shake;
        return 0;
    }
};
