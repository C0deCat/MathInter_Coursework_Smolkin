#include "pch.h"
#include "MathCore/VectorMatrix.h"

namespace mathcore {

    static void ensureScalar(const Value& v) {
        if (!isScalar(v.kind())) throw EvalError("Ожидался скаляр (рациональный или комплексный).");
    }

    VectorValue::VectorValue(std::vector<ValuePtr> items) : m_items(std::move(items)) {
        for (auto& x : m_items) {
            if (!x) throw EvalError("Вектор содержит пустой элемент.");
            ensureScalar(*x);
        }
    }

    std::string VectorValue::toString() const {
        std::ostringstream oss;
        oss << "[ ";
        for (size_t i = 0; i < m_items.size(); ++i) {
            if (i) oss << " ";
            oss << m_items[i]->toString();
        }
        oss << " ]";
        return oss.str();
    }

    ValuePtr VectorValue::add(const Value& rhs) const {
        if (rhs.kind() != ValueKind::Vector) return Value::add(rhs);
        auto& v = static_cast<const VectorValue&>(rhs);
        if (v.items().size() != m_items.size()) throw EvalError("Нельзя сложить векторы разных размеров.");
        std::vector<ValuePtr> out;
        out.reserve(m_items.size());
        for (size_t i = 0; i < m_items.size(); ++i) out.push_back(scalarAdd(*m_items[i], *v.items()[i]));
        return std::make_shared<VectorValue>(std::move(out));
    }

    ValuePtr VectorValue::sub(const Value& rhs) const {
        if (rhs.kind() != ValueKind::Vector) return Value::sub(rhs);
        auto& v = static_cast<const VectorValue&>(rhs);
        if (v.items().size() != m_items.size()) throw EvalError("Нельзя вычесть векторы разных размеров.");
        std::vector<ValuePtr> out;
        out.reserve(m_items.size());
        for (size_t i = 0; i < m_items.size(); ++i) out.push_back(scalarSub(*m_items[i], *v.items()[i]));
        return std::make_shared<VectorValue>(std::move(out));
    }

    ValuePtr VectorValue::mul(const Value& rhs) const {
        if (!isScalar(rhs.kind())) return Value::mul(rhs);
        std::vector<ValuePtr> out;
        out.reserve(m_items.size());
        for (auto& x : m_items) out.push_back(scalarMul(*x, rhs));
        return std::make_shared<VectorValue>(std::move(out));
    }

    ValuePtr VectorValue::div(const Value& rhs) const {
        if (!isScalar(rhs.kind())) return Value::div(rhs);
        std::vector<ValuePtr> out;
        out.reserve(m_items.size());
        for (auto& x : m_items) out.push_back(scalarDiv(*x, rhs));
        return std::make_shared<VectorValue>(std::move(out));
    }

    MatrixValue::MatrixValue(std::vector<std::vector<ValuePtr>> rows) : m_rows(std::move(rows)) {
        if (m_rows.empty()) throw EvalError("Матрица не может быть пустой.");
        const size_t c = m_rows[0].size();
        if (c == 0) throw EvalError("Матрица не может иметь 0 столбцов.");
        for (auto& r : m_rows) {
            if (r.size() != c) throw EvalError("Все строки матрицы должны иметь одинаковую длину.");
            for (auto& x : r) {
                if (!x) throw EvalError("Матрица содержит пустой элемент.");
                ensureScalar(*x);
            }
        }
    }

    std::string MatrixValue::toString() const {
        std::ostringstream oss;
        oss << "[\n";
        for (size_t i = 0; i < m_rows.size(); ++i) {
            if (i) oss << ";\n";
            for (size_t j = 0; j < m_rows[i].size(); ++j) {
                if (j) oss << " ";
                oss << m_rows[i][j]->toString();
            }
        }
        oss << "\n]";
        return oss.str();
    }

    ValuePtr MatrixValue::add(const Value& rhs) const {
        if (rhs.kind() != ValueKind::Matrix) return Value::add(rhs);
        auto& m = static_cast<const MatrixValue&>(rhs);
        if (rows() != m.rows() || cols() != m.cols()) throw EvalError("Нельзя сложить матрицы разных размеров.");
        std::vector<std::vector<ValuePtr>> out(rows(), std::vector<ValuePtr>(cols()));
        for (size_t i = 0; i < rows(); ++i)
            for (size_t j = 0; j < cols(); ++j)
                out[i][j] = scalarAdd(*m_rows[i][j], *m.data()[i][j]);
        return std::make_shared<MatrixValue>(std::move(out));
    }

    ValuePtr MatrixValue::sub(const Value& rhs) const {
        if (rhs.kind() != ValueKind::Matrix) return Value::sub(rhs);
        auto& m = static_cast<const MatrixValue&>(rhs);
        if (rows() != m.rows() || cols() != m.cols()) throw EvalError("Нельзя вычесть матрицы разных размеров.");
        std::vector<std::vector<ValuePtr>> out(rows(), std::vector<ValuePtr>(cols()));
        for (size_t i = 0; i < rows(); ++i)
            for (size_t j = 0; j < cols(); ++j)
                out[i][j] = scalarSub(*m_rows[i][j], *m.data()[i][j]);
        return std::make_shared<MatrixValue>(std::move(out));
    }

    ValuePtr MatrixValue::mul(const Value& rhs) const {
        // Matrix * Scalar
        if (isScalar(rhs.kind())) {
            std::vector<std::vector<ValuePtr>> out(rows(), std::vector<ValuePtr>(cols()));
            for (size_t i = 0; i < rows(); ++i)
                for (size_t j = 0; j < cols(); ++j)
                    out[i][j] = scalarMul(*m_rows[i][j], rhs);
            return std::make_shared<MatrixValue>(std::move(out));
        }

        // Matrix * Vector
        if (rhs.kind() == ValueKind::Vector) {
            auto& v = static_cast<const VectorValue&>(rhs);
            if (cols() != v.items().size()) throw EvalError("Нельзя умножить: число столбцов матрицы не равно размеру вектора.");
            std::vector<ValuePtr> out(rows());
            for (size_t i = 0; i < rows(); ++i) {
                // sum_j a[i][j] * v[j]
                ValuePtr acc = RationalValue::create(0);
                for (size_t j = 0; j < cols(); ++j) {
                    auto prod = scalarMul(*m_rows[i][j], *v.items()[j]);
                    acc = scalarAdd(*acc, *prod);
                }
                out[i] = acc;
            }
            return std::make_shared<VectorValue>(std::move(out));
        }

        // Matrix * Matrix
        if (rhs.kind() == ValueKind::Matrix) {
            auto& b = static_cast<const MatrixValue&>(rhs);
            if (cols() != b.rows()) throw EvalError("Нельзя умножить матрицы: A.cols != B.rows.");
            std::vector<std::vector<ValuePtr>> out(rows(), std::vector<ValuePtr>(b.cols()));
            for (size_t i = 0; i < rows(); ++i) {
                for (size_t k = 0; k < b.cols(); ++k) {
                    ValuePtr acc = RationalValue::create(0);
                    for (size_t j = 0; j < cols(); ++j) {
                        auto prod = scalarMul(*m_rows[i][j], *b.data()[j][k]);
                        acc = scalarAdd(*acc, *prod);
                    }
                    out[i][k] = acc;
                }
            }
            return std::make_shared<MatrixValue>(std::move(out));
        }

        return Value::mul(rhs);
    }

    ValuePtr MatrixValue::div(const Value& rhs) const {
        if (!isScalar(rhs.kind())) return Value::div(rhs);
        std::vector<std::vector<ValuePtr>> out(rows(), std::vector<ValuePtr>(cols()));
        for (size_t i = 0; i < rows(); ++i)
            for (size_t j = 0; j < cols(); ++j)
                out[i][j] = scalarDiv(*m_rows[i][j], rhs);
        return std::make_shared<MatrixValue>(std::move(out));
    }

    ValuePtr MatrixValue::transpose() const {
        std::vector<std::vector<ValuePtr>> out(cols(), std::vector<ValuePtr>(rows()));
        for (size_t i = 0; i < rows(); ++i)
            for (size_t j = 0; j < cols(); ++j)
                out[j][i] = m_rows[i][j];
        return std::make_shared<MatrixValue>(std::move(out));
    }

} // namespace mathcore
