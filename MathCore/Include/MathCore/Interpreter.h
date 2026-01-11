#pragma once
#include "MathCore/Value.h"
#include "MathCore/Tokenizer.h"
#include "MathCore/Errors.h"
#include "MathCore/VectorMatrix.h"
#include "MathCore/RationalValue.h"
#include "MathCore/ComplexValue.h"

#include <map>
#include <optional>
#include <string>

namespace mathcore {

    struct Context {
        std::map<std::string, ValuePtr> vars;
    };

    class Interpreter {
    public:
        Interpreter();

        // Выполняет одну строку: либо присваивание, либо выражение.
        // Возвращает значение выражения, если строка не присваивание.
        std::optional<ValuePtr> executeLine(const std::string& line);

        // Доступ к контексту (например, для тестов)
        const Context& ctx() const { return m_ctx; }

    private:
        // Parser
        ValuePtr parseExpr(Tokenizer& tz);
        ValuePtr parseTerm(Tokenizer& tz);
        ValuePtr parseFactor(Tokenizer& tz);

        ValuePtr parsePrimary(Tokenizer& tz);
        ValuePtr parseVectorOrMatrix(Tokenizer& tz);
        ValuePtr parseFunctionCall(Tokenizer& tz, const std::string& name);

        ValuePtr parseNumber(const Token& tok);

        void expect(Tokenizer& tz, TokType t, const char* msg);

        Context m_ctx;
    };

} // namespace mathcore
