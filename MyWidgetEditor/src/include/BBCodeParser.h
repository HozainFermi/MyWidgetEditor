#pragma once
#include <vector>
#include <string>
#include <stack>
#include <stdexcept>


// Битовые флаги для эффектов
enum TextEffects_ {
    TextEffect_None = 0,
    TextEffect_Bold = 1 << 0,
    TextEffect_Italic = 1 << 1,
    TextEffect_Wave = 1 << 2,
    TextEffect_Rainbow = 1 << 3
};

struct TextSegment {
    std::string content; // Текст без тегов
    uint32_t flags;      // Активные эффекты 
};


//TODO: Два типа флагов, флаги эффектов Rainbow , shake, wave не могут пересекаться,
//а флаги стилей b i могу пересекаться

class BBCodeParser {
public:
    static std::vector<TextSegment> Parse(const std::string& input) {
        std::vector<TextSegment> result;
        std::stack<std::string> activeTags;
        uint32_t currentFlags = TextEffect_None;

        std::string plainText;
        std::string currentTag;
        bool insideTag = false;
        for (size_t i = 0; i < input.length(); ++i) {
            if (input[i] == '[') {

                // Перед тем как обрабатывать тег, сохраняем накопленный текст
                if (!plainText.empty()) {
                    result.push_back({ plainText, currentFlags });
                    plainText.clear();
                }
                insideTag = true;
                currentTag.clear();
                continue;
            }

            if (insideTag) {
                if (input[i] == ']') {
                    insideTag = false;
                    ProcessTag(currentTag, activeTags, currentFlags);
                }
                else {
                    currentTag += input[i];
                }
                continue;
            }

            // Обычный символ
            plainText += input[i];
        }

        // Добавляем остаток текста
        if (!plainText.empty()) {
            result.push_back({ plainText, currentFlags });
        }

        return result;
    }

private:
    static void ProcessTag(const std::string& tag, std::stack<std::string>& stack, uint32_t& flags) {
        if (tag[0] == '/') { // Закрывающий тег [/tag]
            if (!stack.empty()) {
                std::string top = stack.top();
                if("/"+top!=tag) 
                {
                    throw std::invalid_argument("Tags are overlaping");
                }
                flags &= ~GetFlagForTag(top); // Убираем бит
                stack.pop();
            }
        }
        else { // Открывающий тег [tag]
            stack.push(tag);
            flags |= GetFlagForTag(tag); // Добавляем бит
        }
    }

    static uint32_t GetFlagForTag(const std::string& tag) {
        if (tag == "b") return TextEffect_Bold;
        if (tag == "i") return TextEffect_Italic;
        if (tag == "wave") return TextEffect_Wave;
        if (tag == "rainbow") return TextEffect_Rainbow;
        return 0;
    }
};
