# 定义库目标名称，项目内统一使用
set(LIB_TARGET math_utils)

# 构建库：自动适配静态/共享库（通过BUILD_SHARED_LIBS控制）
add_library(${LIB_TARGET}
  math_utils.cpp
)

add_library(MyDemo::${LIB_TARGET} ALIAS math_utils)

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