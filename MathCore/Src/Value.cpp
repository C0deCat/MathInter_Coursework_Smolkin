#include "pch.h"
#include "MathCore/Value.h"
#include "MathCore/Errors.h"

namespace mathcore {

	ValuePtr Value::add(const Value&) const { throw EvalError("Операция '+' не поддерживается для данных типов."); }
	ValuePtr Value::sub(const Value&) const { throw EvalError("Операция '-' не поддерживается для данных типов."); }
	ValuePtr Value::mul(const Value&) const { throw EvalError("Операция '*' не поддерживается для данных типов."); }
	ValuePtr Value::div(const Value&) const { throw EvalError("Операция '/' не поддерживается для данных типов."); }
	ValuePtr Value::transpose() const { throw EvalError("Операция 'T' (транспонирование) не поддерживается для данного типа."); }

} // namespace mathcore
