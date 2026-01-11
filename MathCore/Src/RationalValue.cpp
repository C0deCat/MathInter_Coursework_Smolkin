#include "pch.h"
#include "MathCore/RationalValue.h"
#include "MathCore/ComplexValue.h"

#include <cmath>

namespace mathcore {

    static int64_t abs64(int64_t x) { return x < 0 ? -x : x; }

    void RationalValue::normalize(int64_t& num, int64_t& den) {
        if (den == 0) throw EvalError("Деление на ноль (знаменатель равен 0).");
        if (den < 0) { den = -den; num = -num; }
        const int64_t g = std::gcd(abs64(num), abs64(den));
        if (g != 0) { num /= g; den /= g; }
    }

    RationalValue::RationalValue(int64_t num, int64_t den) : m_num(num), m_den(den) {
        normalize(m_num, m_den);
    }

    ValuePtr RationalValue::create(int64_t num, int64_t den) {
        return std::make_shared<RationalValue>(num, den);
    }

    std::string RationalValue::toString() const {
        // normalize() уже гарантирует: m_den > 0 и дробь сокращена.
        if (m_den == 1) return std::to_string(m_num);

        const bool neg = (m_num < 0);

        // Безопасное взятие модуля для INT64_MIN
        const uint64_t un = neg
            ? (static_cast<uint64_t>(-(m_num + 1)) + 1ULL)
            : static_cast<uint64_t>(m_num);

        const uint64_t ud = static_cast<uint64_t>(m_den);

        const uint64_t whole = un / ud;
        const uint64_t rem = un % ud;

        // Делится нацело -> просто целое число
        if (rem == 0) {
            const std::string s = std::to_string(whole);
            return neg ? ("-" + s) : s;
        }

        // Правильная дробь (целая часть 0) -> "-a/b" или "a/b"
        if (whole == 0) {
            const std::string s = std::to_string(rem) + "/" + std::to_string(ud);
            return neg ? ("-" + s) : s;
        }

        // Смешанная дробь -> "-q r/d" или "q r/d"
        const std::string s =
            std::to_string(whole) + "+(" + std::to_string(rem) + "/" + std::to_string(ud) + ")";

        return neg ? ("-" + s) : s;
    }

    static double toDouble(const RationalValue& r) {
        return static_cast<double>(r.num()) / static_cast<double>(r.den());
    }

    ValuePtr RationalValue::add(const Value& rhs) const {
        if (rhs.kind() == ValueKind::Rational) {
            auto& r = static_cast<const RationalValue&>(rhs);
            // a/b + c/d = (ad + cb)/bd
            const int64_t n = m_num * r.den() + r.num() * m_den;
            const int64_t d = m_den * r.den();
            return RationalValue::create(n, d);
        }
        if (rhs.kind() == ValueKind::Complex) {
            return ComplexValue::create(toDouble(*this), 0.0)->add(rhs);
        }
        return Value::add(rhs);
    }

    ValuePtr RationalValue::sub(const Value& rhs) const {
        if (rhs.kind() == ValueKind::Rational) {
            auto& r = static_cast<const RationalValue&>(rhs);
            const int64_t n = m_num * r.den() - r.num() * m_den;
            const int64_t d = m_den * r.den();
            return RationalValue::create(n, d);
        }
        if (rhs.kind() == ValueKind::Complex) {
            return ComplexValue::create(toDouble(*this), 0.0)->sub(rhs);
        }
        return Value::sub(rhs);
    }

    ValuePtr RationalValue::mul(const Value& rhs) const {
        if (rhs.kind() == ValueKind::Rational) {
            auto& r = static_cast<const RationalValue&>(rhs);
            return RationalValue::create(m_num * r.num(), m_den * r.den());
        }
        if (rhs.kind() == ValueKind::Complex) {
            return ComplexValue::create(toDouble(*this), 0.0)->mul(rhs);
        }
        return Value::mul(rhs);
    }

    ValuePtr RationalValue::div(const Value& rhs) const {
        if (rhs.kind() == ValueKind::Rational) {
            auto& r = static_cast<const RationalValue&>(rhs);
            if (r.num() == 0) throw EvalError("Деление на ноль.");
            return RationalValue::create(m_num * r.den(), m_den * r.num());
        }
        if (rhs.kind() == ValueKind::Complex) {
            return ComplexValue::create(toDouble(*this), 0.0)->div(rhs);
        }
        return Value::div(rhs);
    }

} // namespace mathcore
