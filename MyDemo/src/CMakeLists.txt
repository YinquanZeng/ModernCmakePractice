# 仅添加子目录，不做任何全局配置，保持模块解耦
add_subdirectory(math)

if(MYDEMO_BUILD_APP)
  add_subdirectory(app)
endif()