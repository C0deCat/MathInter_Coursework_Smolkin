#include "pch.h"
#include "MathCore/Tokenizer.h"
#include <cctype>

namespace mathcore {

    Tokenizer::Tokenizer(std::string src) : m_src(std::move(src)) {
        lex();
    }

    void Tokenizer::push(TokType t, std::string text, int line, int col) {
        m_tokens.push_back(Token{ t, std::move(text), line, col });
    }

    void Tokenizer::lex() {
        int line = 1, col = 1;
        for (size_t i = 0; i < m_src.size();) {
            unsigned char ch = static_cast<unsigned char>(m_src[i]);

            if (ch == '\n') { ++line; col = 1; ++i; continue; }
            if (std::isspace(ch)) { ++col; ++i; continue; }

            const int startCol = col;

            switch (ch) {
            case '[': push(TokType::LBracket, "[", line, startCol); ++i; ++col; continue;
            case ']': push(TokType::RBracket, "]", line, startCol); ++i; ++col; continue;
            case '(': push(TokType::LParen, "(", line, startCol); ++i; ++col; continue;
            case ')': push(TokType::RParen, ")", line, startCol); ++i; ++col; continue;
            case ';': push(TokType::Semicolon, ";", line, startCol); ++i; ++col; continue;
            case '+': push(TokType::Plus, "+", line, startCol); ++i; ++col; continue;
            case '-': push(TokType::Minus, "-", line, startCol); ++i; ++col; continue;
            case '*': push(TokType::Star, "*", line, startCol); ++i; ++col; continue;
            case '/': push(TokType::Slash, "/", line, startCol); ++i; ++col; continue;
            case '=': push(TokType::Equal, "=", line, startCol); ++i; ++col; continue;
            default: break;
            }

            // number: digits [ '.' digits ]
            if (std::isdigit(ch) || ch == '.') {
                size_t j = i;
                bool seenDot = false;
                if (m_src[j] == '.') { seenDot = true; ++j; }
                while (j < m_src.size() && std::isdigit(static_cast<unsigned char>(m_src[j]))) ++j;
                if (j < m_src.size() && m_src[j] == '.' && !seenDot) {
                    seenDot = true;
                    ++j;
                    while (j < m_src.size() && std::isdigit(static_cast<unsigned char>(m_src[j]))) ++j;
                }
                auto txt = m_src.substr(i, j - i);
                push(TokType::Number, txt, line, startCol);
                col += static_cast<int>(j - i);
                i = j;
                continue;
            }

            // ident: [A-Za-z_][A-Za-z0-9_]*
            if (std::isalpha(ch) || ch == '_') {
                size_t j = i;
                while (j < m_src.size()) {
                    unsigned char c = static_cast<unsigned char>(m_src[j]);
                    if (!(std::isalnum(c) || c == '_')) break;
                    ++j;
                }
                auto txt = m_src.substr(i, j - i);
                push(TokType::Ident, txt, line, startCol);
                col += static_cast<int>(j - i);
                i = j;
                continue;
            }

            throw ParseError(line, startCol, "Недопустимый символ во входной строке.");
        }

        push(TokType::End, "", line, col);
    }

    Token Tokenizer::next() {
        if (m_pos >= m_tokens.size()) return Token{};
        return m_tokens[m_pos++];
    }

    bool Tokenizer::match(TokType t) {
        if (peek().type == t) { next(); return true; }
        return false;
    }

} // namespace mathcore
