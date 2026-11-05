#pragma once
#include <string>
#include <vector>
#include <json.hpp>

namespace core {

    enum class ConditionOp {
        Equals,       // ==
        NotEquals,    // !=
        Greater,      // >
        GreaterEq,    // >=
        Less,         // <
        LessEq,       // <=
        Contains,     // текст содержит
        NotContains,  // текст не содержит
        Exists,       // колонка есть
        Missing       // колонки нет
    };

    // Одно условие над строкой данных: "<колонка> <оператор> <значение>", например "price > 100".
    // Значение - строка: если и ячейка, и значение похожи на числа, сравниваются как числа
    struct Condition {
        std::string column;
        ConditionOp op = ConditionOp::Equals;
        std::string value;

        bool Matches(const nlohmann::json& row) const;
        nlohmann::json ToJson() const;
        void FromJson(const nlohmann::json& json);
    };

    // Несколько условий через И (match_all) или ИЛИ
    struct ConditionGroup {
        std::vector<Condition> conditions;
        bool match_all = true;

        // Пустая группа пропускает всё
        bool Matches(const nlohmann::json& row) const;
        nlohmann::json ToJson() const;
        void FromJson(const nlohmann::json& json);
        // Редактор условий для панели свойств. true - что-то изменили
        bool RenderEditor();
    };
}
