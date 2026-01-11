#include "pch.h"
#include "MathCore/Interpreter.h"

#include <cctype>

namespace mathcore {

    Interpreter::Interpreter() {
        // Встроенная константа i = 0 + 1i
        m_ctx.vars["i"] = ComplexValue::create(0.0, 1.0);
    }

    static bool isAssignStart(const Token& t1, const Token& t2) {
        return t1.type == TokType::Ident && t2.type == TokType::Equal;
    }

    std::optional<ValuePtr> Interpreter::executeLine(const std::string& line) {
        Tokenizer tz(line);

        // Пустая строка
        if (tz.peek().type == TokType::End) return std::nullopt;

        // Присваивание: IDENT '=' expr
        Token t1 = tz.peek();
        Token t2;
        {
            Tokenizer tz2(line);
            t1 = tz2.next();
            t2 = tz2.peek();
        }

        if (isAssignStart(t1, t2)) {
            const std::string name = t1.text;
            tz.next(); // ident
            tz.next(); // '='
            auto v = parseExpr(tz);
            if (tz.peek().type != TokType::End) throw ParseError(tz.peek().line, tz.peek().col, "Лишние токены в конце строки.");
            m_ctx.vars[name] = v;
            return std::nullopt;
        }

        // Иначе — просто выражение
        auto v = parseExpr(tz);
        if (tz.peek().type != TokType::End) throw ParseError(tz.peek().line, tz.peek().col, "Лишние токены в конце строки.");
        return v;
    }

    void Interpreter::expect(Tokenizer& tz, TokType t, const char* msg) {
        if (!tz.match(t)) {
            const auto& p = tz.peek();
            throw ParseError(p.line, p.col, msg);
        }
    }

    // expr := term (('+'|'-') term)*
    ValuePtr Interpreter::parseExpr(Tokenizer& tz) {
        auto left = parseTerm(tz);
        while (true) {
            if (tz.match(TokType::Plus)) {
                auto right = parseTerm(tz);
                left = left->add(*right);
            }
            else if (tz.match(TokType::Minus)) {
                auto right = parseTerm(tz);
                left = left->sub(*right);
            }
            else break;
        }
        return left;
    }

    // term := factor (('*'|'/') factor)*
    ValuePtr Interpreter::parseTerm(Tokenizer& tz) {
        auto left = parseFactor(tz);
        while (true) {
            if (tz.match(TokType::Star)) {
                auto right = parseFactor(tz);

                // Поддержка Scalar*Vector и Scalar*Matrix
                if (isScalar(left->kind()) && right->kind() == ValueKind::Vector) {
                    left = right->mul(*left);
                }
                else if (isScalar(left->kind()) && right->kind() == ValueKind::Matrix) {
                    left = right->mul(*left);
                }
                else {
                    left = left->mul(*right);
                }
            }
            else if (tz.match(TokType::Slash)) {
                auto right = parseFactor(tz);
                left = left->div(*right);
            }
            else break;
        }
        return left;
    }

    // factor := '-' factor | primary
    ValuePtr Interpreter::parseFactor(Tokenizer& tz) {
        if (tz.match(TokType::Minus)) {
            auto v = parseFactor(tz);
            // 0 - v
            auto zero = RationalValue::create(0);
            return zero->sub(*v);
        }
        return parsePrimary(tz);
    }

    ValuePtr Interpreter::parsePrimary(Tokenizer& tz) {
        const auto& t = tz.peek();

        if (tz.match(TokType::Number)) {
            return parseNumber(t);
        }

        if (tz.match(TokType::Ident)) {
            // function call: IDENT '(' expr ')'
            if (tz.peek().type == TokType::LParen) {
                return parseFunctionCall(tz, t.text);
            }

            // variable
            auto it = m_ctx.vars.find(t.text);
            if (it == m_ctx.vars.end()) throw EvalError("Неизвестная переменная: " + t.text);
            return it->second;
        }

        if (tz.match(TokType::LParen)) {
            auto v = parseExpr(tz);
            expect(tz, TokType::RParen, "Ожидалась ')'.");
            return v;
        }

        if (tz.match(TokType::LBracket)) {
            return parseVectorOrMatrix(tz);
        }

        throw ParseError(t.line, t.col, "Ожидалось выражение.");
    }

    ValuePtr Interpreter::parseFunctionCall(Tokenizer& tz, const std::string& name) {
        expect(tz, TokType::LParen, "Ожидалась '('.");
        auto arg = parseExpr(tz);
        expect(tz, TokType::RParen, "Ожидалась ')'.");
        if (name == "T") return arg->transpose();
        throw EvalError("Неизвестная функция: " + name);
    }

    static bool hasDot(const std::string& s) {
        for (char c : s) if (c == '.') return true;
        return false;
    }

    ValuePtr Interpreter::parseNumber(const Token& tok) {
        // Поддержка десятичных как рациональных: 3.25 = 325/100 -> 13/4
        const std::string& s = tok.text;

        if (!hasDot(s)) {
            // int64
            int64_t n = 0;
            bool neg = false;
            size_t i = 0;
            if (i < s.size() && s[i] == '+') ++i;
            if (i < s.size() && s[i] == '-') { neg = true; ++i; }
            for (; i < s.size(); ++i) {
                if (!std::isdigit(static_cast<unsigned char>(s[i])))
                    throw ParseError(tok.line, tok.col, "Некорректное число.");
                n = n * 10 + (s[i] - '0');
            }
            if (neg) n = -n;
            return RationalValue::create(n);
        }

        // decimal -> rational
        // form: [digits]? '.' digits
        std::string a, b;
        size_t p = s.find('.');
        a = (p == 0) ? "0" : s.substr(0, p);
        b = s.substr(p + 1);
        if (b.empty()) b = "0";

        bool neg = false;
        if (!a.empty() && a[0] == '-') { neg = true; a = a.substr(1); }
        if (a.empty()) a = "0";

        int64_t intPart = 0;
        for (char c : a) {
            if (!std::isdigit(static_cast<unsigned char>(c))) throw ParseError(tok.line, tok.col, "Некорректное число.");
            intPart = intPart * 10 + (c - '0');
        }

        int64_t fracPart = 0;
        int64_t den = 1;
        for (char c : b) {
            if (!std::isdigit(static_cast<unsigned char>(c))) throw ParseError(tok.line, tok.col, "Некорректное число.");
            fracPart = fracPart * 10 + (c - '0');
            den *= 10;
        }

        int64_t num = intPart * den + fracPart;
        if (neg) num = -num;
        return RationalValue::create(num, den);
    }

    ValuePtr Interpreter::parseVectorOrMatrix(Tokenizer& tz) {
        // Внутри: элементы разделяются пробелами, строки матрицы отделяются ';'
        // Пример: [ 1 0; 0 1 ]
        // Закрывающая ']' уже НЕ съедена (мы съели '[' до вызова)
        std::vector<std::vector<ValuePtr>> rows;
        rows.push_back({});

        while (true) {
            if (tz.peek().type == TokType::RBracket) {
                tz.next(); // ]
                break;
            }

            if (tz.match(TokType::Semicolon)) {
                rows.push_back({});
                continue;
            }

            auto v = parseExpr(tz);
            if (!isScalar(v->kind())) throw EvalError("Элемент вектора/матрицы должен быть скаляром.");
            rows.back().push_back(v);
        }

        // Удалим возможную пустую последнюю строку
        if (!rows.empty() && rows.back().empty()) rows.pop_back();
        if (rows.empty()) throw EvalError("Пустой литерал матрицы/вектора.");

        // Если одна строка — это вектор
        if (rows.size() == 1) {
            return std::make_shared<VectorValue>(rows[0]);
        }
        return std::make_shared<MatrixValue>(rows);
    }

} // namespace mathcore
