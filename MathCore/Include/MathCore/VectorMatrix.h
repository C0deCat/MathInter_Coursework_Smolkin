#pragma once
#include "MathCore/Value.h"
#include "MathCore/Errors.h"
#include "MathCore/RationalValue.h"
#include "MathCore/ComplexValue.h"

#include <vector>
#include <string>
#include <sstream>

namespace mathcore {

    inline bool isScalar(ValueKind k) { return k == ValueKind::Rational || k == ValueKind::Complex; }

    inline ValuePtr scalarMul(const Value& a, const Value& b) { return a.mul(b); }
    inline ValuePtr scalarDiv(const Value& a, const Value& b) { return a.div(b); }
    inline ValuePtr scalarAdd(const Value& a, const Value& b) { return a.add(b); }
    inline ValuePtr scalarSub(const Value& a, const Value& b) { return a.sub(b); }

    class VectorValue final : public Value {
    public:
        explicit VectorValue(std::vector<ValuePtr> items);

        ValueKind kind() const override { return ValueKind::Vector; }
        std::string toString() const override;

        const std::vector<ValuePtr>& items() const { return m_items; }

        ValuePtr add(const Value& rhs) const override;
        ValuePtr sub(const Value& rhs) const override;
        ValuePtr mul(const Value& rhs) const override; // * scalar
        ValuePtr div(const Value& rhs) const override; // / scalar

    private:
        std::vector<ValuePtr> m_items;
    };

    class MatrixValue final : public Value {
    public:
        explicit MatrixValue(std::vector<std::vector<ValuePtr>> rows);

        ValueKind kind() const override { return ValueKind::Matrix; }
        std::string toString() const override;

        size_t rows() const { return m_rows.size(); }
        size_t cols() const { return m_rows.empty() ? 0 : m_rows[0].size(); }
        const std::vector<std::vector<ValuePtr>>& data() const { return m_rows; }

        ValuePtr add(const Value& rhs) const override;
        ValuePtr sub(const Value& rhs) const override;
        ValuePtr mul(const Value& rhs) const override; // * scalar / vector / matrix
        ValuePtr div(const Value& rhs) const override; // / scalar
        ValuePtr transpose() const override;

    private:
        std::vector<std::vector<ValuePtr>> m_rows;
    };

} // namespace mathcore
