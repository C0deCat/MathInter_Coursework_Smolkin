#pragma once
#include "MathCore/Value.h"
#include "MathCore/Errors.h"

#include <complex>
#include <string>

namespace mathcore {

    class ComplexValue final : public Value {
    public:
        static ValuePtr create(double re, double im);

        ValueKind kind() const override { return ValueKind::Complex; }
        std::string toString() const override;

        std::complex<double> value() const { return m_v; }

        ValuePtr add(const Value& rhs) const override;
        ValuePtr sub(const Value& rhs) const override;
        ValuePtr mul(const Value& rhs) const override;
        ValuePtr div(const Value& rhs) const override;

    public:
        explicit ComplexValue(std::complex<double> v) : m_v(v) {}
        std::complex<double> m_v{};
    };

} // namespace mathcore
