#include "MyStd/Function.h"
#include "MyStd/Forward.h"

#include <exception>
#include <gtest/gtest.h>

#include <iostream>

using namespace mystd;
TEST(Function, Function) {
  Function<int(int, int)> f;
  EXPECT_THROW(f(1, 2), std::runtime_error);
  f = [](int a, int b) { return a + b; };
  EXPECT_EQ(f(1, 2), 3);
}
