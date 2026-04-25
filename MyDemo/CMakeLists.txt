# 最低CMake版本要求，兼容3.15到最新稳定版，覆盖主流Linux/Windows/macOS环境
cmake_minimum_required(VERSION 3.15...3.30)

# 项目声明：指定名称、版本、编程语言
project(MyDemo
  VERSION 1.0.0
  DESCRIPTION "Modern CMake Best Practice Demo"
  LANGUAGES CXX
)

# 全局基础配置（仅设置不可变的全局规则，不污染目标编译选项）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)     # 生成编译命令，对LSP有用
set(CMAKE_CXX_STANDARD 17)                # C++17标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)       # 强制要求标准支持，不回退
set(CMAKE_CXX_EXTENSIONS OFF)              # 关闭编译器非标准扩展，保证跨平台兼容
set(CMAKE_BUILD_TYPE Release CACHE STRING "Default build type") # 默认构建类型

# 项目功能开关，方便下游集成时按需裁剪
option(MYDEMO_BUILD_TESTS "Build unit tests" ON) # 注意网络环境
option(MYDEMO_BUILD_APP "Build demo executable" ON)
option(MYDEMO_INSTALL "Enable install rules" ON)

# 引入GNU标准安装目录，跨平台统一安装路径（lib/bin/include等）
include(GNUInstallDirs)
# 引入包配置辅助工具，用于生成find_package兼容文件
include(CMakePackageConfigHelpers)


# 添加源码子目录，核心构建逻辑下沉到子模块
add_subdirectory(src)

# 按需构建单元测试
message("MYDEMO_BUILD_TESTS: ${MYDEMO_BUILD_TESTS}")
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