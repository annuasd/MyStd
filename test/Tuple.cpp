#include "MyStd/Tuple.h"

#include <gtest/gtest.h>

#include <iostream>
#include <tuple>

class A {};
class B {
  int i;

public:
  B(int i) : i(i) {}
};

using namespace mystd;

TEST(Tuple, Tuple) {
  // 测试空基类优化
  struct Empty {};
  struct NonEmpty {
    int x;
  };
  // 测试空类型优化效果
  Tuple<Empty, int> t1;
  Tuple<NonEmpty, int> t2;

  // 验证空基类优化是否生效
  EXPECT_LT(sizeof(t1), sizeof(t2));

  // 测试get方法
  Tuple<int, std::string, Empty> t3(42, "test", Empty{});

  // 验证get方法正确性
  EXPECT_EQ(t3.get<0>(), 42);
  EXPECT_EQ(t3.get<1>(), "test");

  // 测试const版本的get
  const auto &t3_const = t3;
  EXPECT_EQ(t3_const.get<0>(), 42);

  // 测试空类型成员的get
  auto &empty = t3.get<2>();
  EXPECT_TRUE((std::is_same_v<decltype(empty), Empty &>));
}
