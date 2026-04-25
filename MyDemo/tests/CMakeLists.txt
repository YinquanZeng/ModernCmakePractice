set(TEST_TARGET math_utils_test)

# 引入FetchContent，源码级集成GoogleTest，无需系统预装
# 注意网络环境！！！
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
  GIT_SHALLOW TRUE # 浅克隆，加速下载
)
# 屏蔽GTest内部警告，不污染本项目编译输出
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 定义测试目标
add_executable(${TEST_TARGET}
  math_test.cpp
)

# 链接测试框架与被测库
target_link_libraries(${TEST_TARGET}
  PRIVATE
    MyDemo::math_utils
    GTest::gtest_main
    GTest::gmock
)

# 添加到CTest测试用例，支持ctest命令执行
include(GoogleTest)
gtest_discover_tests(${TEST_TARGET})