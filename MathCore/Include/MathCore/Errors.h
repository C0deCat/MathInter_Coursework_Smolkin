#pragma once
#include <stdexcept>
#include <string>

namespace mathcore {

    struct ParseError : public std::runtime_error {
        int line;
        int col;
        explicit ParseError(int line_, int col_, const std::string& msg)
            : std::runtime_error(msg), line(line_), col(col_) {}
    };

    struct EvalError : public std::runtime_error {
        explicit EvalError(const std::string& msg) : std::runtime_error(msg) {}
    };

} // namespace mathcore
