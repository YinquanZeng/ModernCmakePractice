# 现代 CMake 最佳实践完整示例

> 可以直接复制本工程中的MyDemo文件夹  
> 在ubuntu22.04中测试通过  

本示例严格遵循**目标导向、作用域隔离、可重定位、跨平台兼容**的现代CMake核心原则，覆盖库构建、可执行文件、单元测试、依赖管理、安装导出全流程，可直接复制运行，也可作为企业级项目模板使用。

## 一、项目标准目录结构

```Plain Text

MyDemo/
├── CMakeLists.txt          # 顶层CMake配置（入口）
├── cmake/                  # CMake模块与配置模板
│   └── MyDemoConfig.cmake.in
├── include/                # 对外公开的API头文件
│   └── MyDemo/
│       └── math_api.h
├── src/                    # 源码实现
│   ├── CMakeLists.txt
│   ├── math/               # 核心库模块
│   │   ├── CMakeLists.txt
│   │   ├── math_utils.h
│   │   └── math_utils.cpp
│   └── app/                # 可执行程序
│       ├── CMakeLists.txt
│       └── main.cpp
├── tests/                  # 单元测试
│   ├── CMakeLists.txt
│   └── math_test.cpp
└── README.md
```

## 二、完整文件内容实现

### 1. 顶层 CMakeLists.txt（核心入口）

```CMake

# 最低CMake版本要求，兼容3.15到最新稳定版，覆盖主流Linux/Windows/macOS环境
cmake_minimum_required(VERSION 3.15...3.30)

# 项目声明：指定名称、版本、编程语言
project(MyDemo
  VERSION 1.0.0
  DESCRIPTION "Modern CMake Best Practice Demo"
  LANGUAGES CXX
)

# 全局基础配置（仅设置不可变的全局规则，不污染目标编译选项）
set(CMAKE_CXX_STANDARD 17)                # C++17标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)       # 强制要求标准支持，不回退
set(CMAKE_CXX_EXTENSIONS OFF)              # 关闭编译器非标准扩展，保证跨平台兼容
set(CMAKE_BUILD_TYPE Release CACHE STRING "Default build type") # 默认构建类型

# 项目功能开关，方便下游集成时按需裁剪
option(MYDEMO_BUILD_TESTS "Build unit tests" ON)
option(MYDEMO_BUILD_APP "Build demo executable" ON)
option(MYDEMO_INSTALL "Enable install rules" ON)

# 引入GNU标准安装目录，跨平台统一安装路径（lib/bin/include等）
include(GNUInstallDirs)
# 引入包配置辅助工具，用于生成find_package兼容文件
include(CMakePackageConfigHelpers)

# 添加源码子目录，核心构建逻辑下沉到子模块
add_subdirectory(src)

# 按需构建单元测试
if(MYDEMO_BUILD_TESTS)
  enable_testing() # 启用CTest测试框架
  add_subdirectory(tests)
endif()

# 安装与导出配置（核心：让下游项目可通过find_package直接集成）
if(MYDEMO_INSTALL)
  # 1. 安装公开头文件，保留目录结构
  install(DIRECTORY include/MyDemo
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
  )

  # 2. 生成包版本文件，兼容语义化版本
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/MyDemoConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion # 主版本号一致即兼容
  )

  # 3. 配置包入口文件
  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MyDemoConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MyDemoConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MyDemo
  )

  # 4. 安装CMake配置文件，让下游find_package可识别
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/MyDemoConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/MyDemoConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MyDemo
  )

  # 5. 安装导出目标，生成带命名空间的导入目标
  install(EXPORT MyDemoTargets
    FILE MyDemoTargets.cmake
    NAMESPACE MyDemo::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MyDemo
  )
endif()
```

### 2. src/CMakeLists.txt（模块调度）

```CMake

# 仅添加子目录，不做任何全局配置，保持模块解耦
add_subdirectory(math)

if(MYDEMO_BUILD_APP)
  add_subdirectory(app)
endif()
```

### 3. 核心库模块 src/math/CMakeLists.txt

```CMake

# 定义库目标名称，项目内统一使用
set(LIB_TARGET math_utils)

# 构建库：自动适配静态/共享库（通过BUILD_SHARED_LIBS控制）
add_library(${LIB_TARGET}
  math_utils.cpp
)

# 【核心最佳实践】头文件路径作用域控制
# PUBLIC：下游目标自动继承该路径，无需重复配置
# BUILD_INTERFACE：构建时使用的路径，INSTALL_INTERFACE：安装后使用的路径，保证可重定位
target_include_directories(${LIB_TARGET}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} # 内部头文件，仅本库可见，不对外暴露
)

# 【核心最佳实践】编译特性与选项，仅对本目标生效
target_compile_features(${LIB_TARGET} PUBLIC cxx_std_17) # 向下游传递C++17要求
set_target_properties(${LIB_TARGET} PROPERTIES
  VERSION ${PROJECT_VERSION}          # 库版本
  SOVERSION ${PROJECT_VERSION_MAJOR}  # 主版本号，用于so版本兼容
  CXX_VISIBILITY_PRESET hidden        # 隐藏非公开符号，减小库体积
  VISIBILITY_INLINES_HIDDEN ON
)

# 【核心最佳实践】链接库作用域控制
# PRIVATE：仅本库使用，不向下游传递；PUBLIC：本库和下游都需要使用
# 示例：若依赖fmt库，可写 target_link_libraries(${LIB_TARGET} PRIVATE fmt::fmt)

# 绑定到项目导出目标，用于后续安装
install(TARGETS ${LIB_TARGET}
  EXPORT MyDemoTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
```

### 4. 可执行程序 src/app/CMakeLists.txt

```CMake

set(APP_TARGET demo_app)

# 定义可执行目标
add_executable(${APP_TARGET}
  main.cpp
)

# 【核心最佳实践】PRIVATE链接，仅本程序使用，不对外暴露任何依赖
target_link_libraries(${APP_TARGET}
  PRIVATE
    MyDemo::math_utils # 直接使用命名空间目标，保证构建顺序与依赖正确
)

# 安装可执行文件
install(TARGETS ${APP_TARGET}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
```

### 5. 单元测试 tests/CMakeLists.txt

```CMake

set(TEST_TARGET math_utils_test)

# 引入FetchContent，源码级集成GoogleTest，无需系统预装
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
```

### 6. CMake配置模板 cmake/[MyDemoConfig.cmake.in](MyDemoConfig.cmake.in)

```CMake

@PACKAGE_INIT@

# 导入项目导出的目标
include("${CMAKE_CURRENT_LIST_DIR}/MyDemoTargets.cmake")

# 检查依赖（若有），示例：find_package(fmt REQUIRED)
check_required_components(MyDemo)
```

### 7. C++源码文件（可直接编译运行）

#### include/MyDemo/math_api.h（公开API）

```cpp

#ifndef MYDEMO_MATH_API_H
#define MYDEMO_MATH_API_H

namespace MyDemo {
namespace Math {

// 对外公开的API接口
int add(int a, int b);
int multiply(int a, int b);

} // namespace Math
} // namespace MyDemo

#endif // MYDEMO_MATH_API_H
```

#### src/math/math_utils.h（内部头文件）

```cpp

#ifndef MYDEMO_MATH_UTILS_H
#define MYDEMO_MATH_UTILS_H

namespace MyDemo {
namespace Math {

// 内部工具函数，不对外暴露
bool is_negative(int num);

} // namespace Math
} // namespace MyDemo

#endif // MYDEMO_MATH_UTILS_H
```

#### src/math/math_utils.cpp（实现）

```cpp

#include "MyDemo/math_api.h"
#include "math_utils.h"

namespace MyDemo {
namespace Math {

int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

bool is_negative(int num) {
    return num < 0;
}

} // namespace Math
} // namespace MyDemo
```

#### src/app/main.cpp（可执行程序）

```cpp

#include "MyDemo/math_api.h"
#include <iostream>

int main() {
    std::cout << "MyDemo v" << MYDEMO_VERSION << std::endl;
    std::cout << "3 + 5 = " << MyDemo::Math::add(3, 5) << std::endl;
    std::cout << "4 * 6 = " << MyDemo::Math::multiply(4, 6) << std::endl;
    return 0;
}
```

#### tests/math_test.cpp（单元测试）

```cpp

#include "MyDemo/math_api.h"
#include <gtest/gtest.h>

TEST(MathAPITest, AddFunction) {
    EXPECT_EQ(MyDemo::Math::add(2, 3), 5);
    EXPECT_EQ(MyDemo::Math::add(-1, 1), 0);
    EXPECT_EQ(MyDemo::Math::add(0, 0), 0);
}

TEST(MathAPITest, MultiplyFunction) {
    EXPECT_EQ(MyDemo::Math::multiply(3, 4), 12);
    EXPECT_EQ(MyDemo::Math::multiply(-2, 5), -10);
    EXPECT_EQ(MyDemo::Math::multiply(0, 100), 0);
}
```

## 三、构建、测试、安装完整命令

### 1. 基础构建（全平台通用）

```Bash

# 1. 配置项目
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# 2. 编译构建（-j 开启多核并行编译）
cmake --build build -j

# 3. 运行单元测试
cd build && ctest --output-on-failure && cd ..

# 4. 运行可执行程序
./build/src/app/demo_app  # Linux/macOS
build\src\app\Release\demo_app.exe  # Windows MSVC
```

### 2. 安装部署

```Bash

# 安装到系统默认路径（Linux:/usr/local Windows:C:\Program Files）
cmake --install build

# 自定义安装路径
cmake --install build --prefix ./install
```

### 3. 下游项目集成示例

安装完成后，其他CMake项目可通过两行代码直接集成：

```CMake

find_package(MyDemo 1.0.0 REQUIRED)
target_link_libraries(your_app PRIVATE MyDemo::math_utils)
```

## 四、核心最佳实践要点总结

1. **目标为中心（Target-based）**

    - 使用 `add_library()` 和 `add_executable()` 明确声明目标。
    - 通过 `target_* `系列命令（如` target_include_directories()、target_link_libraries()`）严格隔离目标作用域，避免全局命令（如 `include_directories/link_directories/add_definitions`）。

2. **精准的作用域控制**：

    - `PUBLIC`：本目标+下游依赖目标都需要使用的属性

    - `PRIVATE`：仅本目标内部使用，不向下游传递

    - `INTERFACE`：仅下游目标使用，本目标编译不依赖

3. **可重定位的安装设计**：使用`BUILD_INTERFACE`/`INSTALL_INTERFACE`分离构建与安装路径，保证库可复制到任意路径使用，无硬编码依赖。

4. **标准的包导出机制**：完整实现`Config.cmake`/`ConfigVersion.cmake`/`Targets.cmake`三件套，完美兼容CMake官方`find_package`机制，下游零成本集成。

5. **规范的依赖管理**：优先使用`find_package`集成系统依赖，使用`FetchContent`源码级集成第三方库，避免手动管理路径与版本。

6. **跨平台兼容设计**：使用`GNUInstallDirs`统一安装路径，通过生成器表达式处理编译器差异，关闭非标准语言扩展，保证Windows/Linux/macOS全平台兼容。

7. **可裁剪的项目结构**：通过`option`控制测试、示例、安装功能的开关，方便作为子模块被下游项目集成。
