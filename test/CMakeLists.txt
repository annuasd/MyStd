enable_testing()


# 递归搜索当前目录下所有的C++源文件
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_executable(unittest ${TEST_SOURCES})
target_link_libraries(
  unittest
  GTest::gtest_main
)


include(GoogleTest)
gtest_discover_tests(unittest)