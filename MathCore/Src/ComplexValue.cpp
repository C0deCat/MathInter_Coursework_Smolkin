#include "pch.h"
#include "MathCore/ComplexValue.h"
#include "MathCore/RationalValue.h"

#include <cmath>
#include <sstream>

namespace mathcore {

    ValuePtr ComplexValue::create(double re, double im) {
        return std::make_shared<ComplexValue>(std::complex<double>(re, im));
    }

    std::string ComplexValue::toString() const {
        const double re = m_v.real();
        const double im = m_v.imag();

        // Упрощённый вывод
        std::ostringstream oss;
        oss.setf(std::ios::fixed);
        oss.precision(10);

        if (std::abs(im) < 1e-12) {
            oss << re;
            return oss.str();
        }
        if (std::abs(re) < 1e-12) {
            oss << im << "i";
            return oss.str();
        }

        oss << re;
        if (im >= 0) oss << "+";
        oss << im << "i";
        return oss.str();
    }

    static std::complex<double> asComplex(const Value& v) {
        if (v.kind() == ValueKind::Complex) return static_cast<const ComplexValue&>(v).value();
        if (v.kind() == ValueKind::Rational) {
            auto& r = static_cast<const RationalValue&>(v);
            return { static_cast<double>(r.num()) / static_cast<double>(r.den()), 0.0 };
        }
        throw EvalError("Ожидался скаляр (рациональный или комплексный).");
    }

    ValuePtr ComplexValue::add(const Value& rhs) const {
        if (rhs.kind() == ValueKind::Rational || rhs.kind() == ValueKind::Complex) {
            return create((m_v + asComplex(rhs)).real(), (m_v + asComplex(rhs)).imag());
        }
        return Value::add(rhs);
    }

    ValuePtr ComplexValue::sub(const Value& rhs) const {
        if (rhs.kind() == ValueKind::Rational || rhs.kind() == ValueKind::Complex) {
            const auto res = m_v - asComplex(rhs);
            return create(res.real(), res.imag());
        }
        return Value::sub(rhs);
    }

    ValuePtr ComplexValue::mul(const Value& rhs) const {
        if (rhs.kind() == ValueKind::Rational || rhs.kind() == ValueKind::Complex) {
            const auto res = m_v * asComplex(rhs);
            return create(res.real(), res.imag());
        }
        return Value::mul(rhs);
    }

    ValuePtr ComplexValue::div(const Value& rhs) const {
        if (rhs.kind() == ValueKind::Rational || rhs.kind() == ValueKind::Complex) {
            const auto d = asComplex(rhs);
            if (std::abs(d.real()) < 1e-18 && std::abs(d.imag()) < 1e-18) throw EvalError("Деление на ноль.");
            const auto res = m_v / d;
            return create(res.real(), res.imag());
        }
        return Value::div(rhs);
    }

} // namespace mathcore
