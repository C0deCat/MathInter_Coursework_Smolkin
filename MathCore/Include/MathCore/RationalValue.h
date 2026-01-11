#pragma once
#include "MathCore/Value.h"
#include "MathCore/Errors.h"

#include <cstdint>
#include <numeric>
#include <string>

namespace mathcore {

    class ComplexValue; // forward

    class RationalValue final : public Value {
    public:
        static ValuePtr create(int64_t num, int64_t den = 1);

        ValueKind kind() const override { return ValueKind::Rational; }
        std::string toString() const override;

        int64_t num() const { return m_num; }
        int64_t den() const { return m_den; }

        ValuePtr add(const Value& rhs) const override;
        ValuePtr sub(const Value& rhs) const override;
        ValuePtr mul(const Value& rhs) const override;
        ValuePtr div(const Value& rhs) const override;

    public:
        RationalValue(int64_t num, int64_t den);

        static void normalize(int64_t& num, int64_t& den);

        int64_t m_num{};
        int64_t m_den{ 1 };
    };

} // namespace mathcore
