#pragma once
#include <memory>
#include <string>

namespace mathcore {

    enum class ValueKind { Rational, Complex, Vector, Matrix };

    class Value;
    using ValuePtr = std::shared_ptr<Value>;

    class Value {
    public:
        virtual ~Value() = default;

        virtual ValueKind kind() const = 0;
        virtual std::string toString() const = 0;

        // Базовые операции: по умолчанию не поддерживаются.
        virtual ValuePtr add(const Value& rhs) const;
        virtual ValuePtr sub(const Value& rhs) const;
        virtual ValuePtr mul(const Value& rhs) const;
        virtual ValuePtr div(const Value& rhs) const;
        virtual ValuePtr transpose() const; // для матриц
    };

} // namespace mathcore
