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