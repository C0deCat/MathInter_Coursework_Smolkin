#include "pch.h"
#include "CppUnitTest.h"

#include "MathCore/Interpreter.h"
#include "MathCore/RationalValue.h"
#include "MathCore/VectorMatrix.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace MathTests {

    TEST_CLASS(RationalTests) {
public:
    TEST_METHOD(Normalization) {
        auto r = mathcore::RationalValue::create(2, 4);
        Assert::AreEqual(std::string("1/2"), r->toString());
    }

    TEST_METHOD(Addition) {
        auto a = mathcore::RationalValue::create(1, 3);
        auto b = mathcore::RationalValue::create(1, 6);
        auto c = a->add(*b);
        Assert::AreEqual(std::string("1/2"), c->toString());
    }
    };

    TEST_CLASS(InterpreterSmokeTests) {
public:
    TEST_METHOD(SampleFromTask) {
        mathcore::Interpreter it;

        it.executeLine("V1 = [ 1 2 3 ]");
        it.executeLine("M1 = [ 1 0 0; 0 1 0; 0 0 1 ]");
        it.executeLine("V2 = M1 * V1");
        it.executeLine("R = 1 / 3");
        it.executeLine("V3 = V2 * R");
        it.executeLine("M2 = T(M1)");

        auto v3 = it.executeLine("V3");
        Assert::IsTrue(v3.has_value());
        Assert::AreEqual(std::string("[ 1/3 2/3 1 ]"), (*v3)->toString());

        auto m2 = it.executeLine("M2");
        Assert::IsTrue(m2.has_value());
        Assert::AreEqual(std::string("[ 1 0 0; 0 1 0; 0 0 1 ]"), (*m2)->toString());
    }
    };

} // namespace MathTests
