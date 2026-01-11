#pragma once
#include "MathCore/Errors.h"
#include <string>
#include <vector>

namespace mathcore {

    enum class TokType {
        End,
        Ident,
        Number,
        LBracket, RBracket,
        LParen, RParen,
        Semicolon,
        Plus, Minus, Star, Slash,
        Equal
    };

    struct Token {
        TokType type{ TokType::End };
        std::string text;
        int line{ 1 };
        int col{ 1 };
    };

    class Tokenizer {
    public:
        explicit Tokenizer(std::string src);

        const Token& peek() const { return m_tokens[m_pos]; }
        Token next();
        bool match(TokType t);

    private:
        void lex();
        void push(TokType t, std::string text, int line, int col);

        std::string m_src;
        std::vector<Token> m_tokens;
        size_t m_pos{ 0 };
    };

} // namespace mathcore
