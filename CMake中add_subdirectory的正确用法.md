# CMake 中 `add_subdirectory` 的正确用法（完整指南）


`add_subdirectory` 是 CMake 组织**多目录项目**的核心命令，**90% 的新手错误都源于用错了它**。我会用最清晰、最实用的方式告诉你：**怎么用、什么时候用、禁忌是什么**。

---

## 一、最核心一句话

`add_subdirectory(dir)`  

= **进入 dir 目录，执行里面的 CMakeLists.txt，然后回来**。

它的作用：

- 拆分大型项目

- 解耦模块（库、工具、测试、示例）

- 构建子目录里的 `target`（库/可执行文件）

---

## 二、正确用法（标准格式）

### 1. 基础用法

```CMake

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(third_party/googletest)
```

### 2. 常用完整写法（指定二进制输出目录）

```CMake

add_subdirectory(
  源目录
  编译输出目录（可选）
)
```

日常几乎不用写第二个参数，CMake 会自动映射。

---

## 三、必须遵守的 4 条黄金规则（最重要）

### 规则 1：**先添加子目录，再使用它的 target**

顺序不能反！

```CMake

# 错误
add_executable(app main.cpp)
target_link_libraries(app PRIVATE mylib)
add_subdirectory(src/mylib) # 晚了！

# 正确
add_subdirectory(src/mylib) # 先生成 mylib
add_executable(app main.cpp)
target_link_libraries(app PRIVATE mylib)
```

---

### 规则 2：**子目录之间不要互相依赖全局变量**

❌ 错误（全局传递）：

```CMake

set(MY_SOURCES a.cpp b.cpp) # 顶层定义
add_subdirectory(src)
```

子目录里直接用 `MY_SOURCES` 极易出错。

✅ 正确（target 传递，现代 CMake 唯一正确方式）：

```CMake

# 子目录创建库
add_library(mylib src1.cpp src2.cpp)

# 顶层直接链接 target，不关心源码
target_link_libraries(app PRIVATE mylib)
```

---

### 规则 3：**不要在子目录里改全局配置**

❌ 禁止在子目录写：

```CMake

set(CMAKE_CXX_FLAGS "-Wall")    # 污染全局
include_directories(...)       # 全局头文件，灾难
link_directories(...)           # 全局链接，灾难
add_definitions(-DXXX)          # 全局宏，灾难
```

✅ 子目录**只允许写 target 相关命令**：

```CMake

add_library(...)
add_executable(...)
target_include_directories(...)
target_link_libraries(...)
target_compile_definitions(...)
```

---

### 规则 4：**一个子目录 = 一个模块 = 一个 target**

最佳结构：

```Plain Text

src/
├── A/
│   ├── CMakeLists.txt  → 生成库 A
│   ├── a.cpp
│   └── a.h
├── B/
│   ├── CMakeLists.txt  → 生成库 B
│   └── ...
```

**一个目录只做一件事，只提供一个库/执行程序。**

---

## 四、最常用的正确结构示例（直接复制）

### 顶层 CMakeLists.txt

```CMake

cmake_minimum_required(VERSION 3.15)
project(MyProject)

# 先构建子模块
add_subdirectory(src/math)   # 生成 math_lib
add_subdirectory(src/app)    # 生成 app
add_subdirectory(tests)      # 生成测试
```

### src/math/CMakeLists.txt

```CMake

add_library(math_lib
  math.cpp
  math.h
)

target_include_directories(math_lib
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
```

### src/app/CMakeLists.txt

```CMake

add_executable(app main.cpp)

# 直接依赖子目录的 target
target_link_libraries(app PRIVATE math_lib)
```

---

## 五、常见错误（新手必看）

### 错误 1：在子目录里用 `../` 找上级文件

```CMake

# 极其不规范、容易崩
add_library(mylib ../common/utils.cpp)
```

✅ 正确：  

把 common 也变成子模块，然后链接它。

```CMake

add_subdirectory(../common)
target_link_libraries(mylib PRIVATE common)
```

---

### 错误 2：多次添加同一个目录

```CMake

add_subdirectory(src)
add_subdirectory(src) # 报错：target 重复
```

---

### 错误 3：在 if 外 add_subdirectory，在 if 内使用

```CMake

add_subdirectory(tests)

if(BUILD_TESTS)
  add_test(...) # 可能找不到 tests 里的 target
endif()
```

---

## 六、高级技巧（工程必备）

### 1. 条件编译子目录

```CMake

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
  add_subdirectory(tests)
endif()
```

### 2. 第三方库用 `EXCLUDE_FROM_ALL`

不让它被默认安装、污染全局：

```CMake

add_subdirectory(third_party/googletest EXCLUDE_FROM_ALL)
```

### 3. 让子目录“看到”顶层的版本号

```CMake

# 顶层
set(VERSION_MAJOR 1)
add_subdirectory(src)

# 子目录
target_compile_definitions(mylib
  PRIVATE VERSION_MAJOR=${VERSION_MAJOR}
)
```

---

## 七、终极总结（记住这 5 条就够）

1. **先 add_subdirectory，再使用 target**  

2. **子目录只创建 target，不修改全局配置**  

3. **模块之间用 target_link_libraries 依赖，不用文件路径互相引用**  

4. **一个目录一个模块**  

5. **永远不用全局命令：include_directories / link_directories**

---
