#include "MyStd/Variant.h"
#include <gtest/gtest.h>
#include <iostream>
#include <type_traits>
#include <variant>

using namespace mystd;
TEST(Variant, VariantIndex) {
  auto idx0 = VariantIndex<Variant<int, double, float>, int>::value;
  auto idx1 = VariantIndex<Variant<int, double, float>, double>::value;
  auto idx2 = VariantIndex<Variant<int, double, float>, float>::value;
  EXPECT_EQ(idx0, 0);
  EXPECT_EQ(idx1, 1);
  EXPECT_EQ(idx2, 2);
}

TEST(Variant, ConvertibleVariantIndex) {
  auto idx0 = VariantConvertibleIndex<
      Variant<int, std::vector<int>, std::vector<float>, float>, float>::value;
  EXPECT_EQ(idx0, 3);
}

TEST(Variant, VariantAlternative) {
  using T = Variant<int, double, float>;
  using Alt0 = VariantAlternative<T, 0>;
  using Alt1 = VariantAlternative<T, 1>;
  using Alt2 = VariantAlternative<T, 2>;
  EXPECT_TRUE((std::is_same_v<int, Alt0::type>));
  EXPECT_TRUE((std::is_same_v<double, Alt1::type>));
  EXPECT_TRUE((std::is_same_v<float, Alt2::type>));
}

struct A {
  int d = 2333;
  int i = 0;
  A(int i) : i(i) {}
  A(A const &other) { i = other.i; }
  A(A &&other) {
    i = other.i;
    other.i = 0;
  }
  A &operator=(const A &other) {
    if (this != &other) {
      i = other.i;
    }
    return *this;
  }
  A &operator=(A &&other) {
    if (this != &other) {
      i = other.i;
      other.i = 0;
    }
    return *this;
  }

  ~A() {}
};

TEST(Variant, Construct) {
  using Ty = Variant<A, float>;
  Ty a(12);
  Ty b(3.2f);
  EXPECT_TRUE(a.get<A>().i == 12);
  EXPECT_THROW(b.get<A>(), BadVariantAccess);
}
TEST(Variant, Copy) {
  using Ty = Variant<A>;
  Ty a(12);
  Ty b(a);
  EXPECT_TRUE(b.get<A>().i == 12);
}
TEST(Variant, Move) {
  using Ty = Variant<A>;
  Ty a(12);
  Ty b(std::move(a));
  EXPECT_TRUE(a.get<A>().i == 0);
  EXPECT_TRUE(b.get<A>().i == 12);
}
TEST(Variant, CopyAssign) {
  using Ty = Variant<A>;
  Ty a(12);
  Ty b(3);
  b = a;
  EXPECT_TRUE(a.get<A>().i == 12);
  EXPECT_TRUE(b.get<A>().i == 12);
}

TEST(Variant, MoveAssign) {
  using Ty = Variant<A>;
  Ty a(12);
  Ty b(3);
  b = std::move(a);
  EXPECT_TRUE(a.get<A>().i == 0);
  EXPECT_TRUE(b.get<A>().i == 12);
}
class B {};

// 定义一个访问器结构体，使用 operator() 重载来处理不同类型
struct VariantVisitor {
  int operator()(int i) const { return 0; }
  int operator()(float i) const { return 1; }
  int operator()(double i) const { return 2; }
};

TEST(Variant, VariantVisitExample) {
  using MyVariant = Variant<int, double, float>;

  MyVariant v1 = 42;
  MyVariant v2 = 3.14;
  MyVariant v3 = 2.718f;

  auto a = v1.accept(VariantVisitor{});
  auto b = v2.accept(VariantVisitor{});
  auto c = v3.accept(VariantVisitor{});
  EXPECT_TRUE(a == 0);
  EXPECT_TRUE(b == 2);
  EXPECT_TRUE(c == 1);
}