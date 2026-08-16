# travel_cpp

用于学习 C++17、CMake 和 GoogleTest 的小型项目。

## 目录结构

```text
travel_cpp/
├── include/
│   └── travel_cpp/       # 对外公开的头文件
│       ├── error_code.h
│       ├── get_input.h
│       └── log.h
├── src/                  # 库实现和程序入口
│   ├── error_code.cpp
│   ├── get_input.cpp
│   ├── log.cpp
│   └── main.cpp
├── tests/                # 测试源码及测试目标
├── CMakeLists.txt        # 定义库、主程序和测试入口
└── build.sh             # 配置、编译、测试脚本
```

`include/travel_cpp/` 而不是直接使用 `include/` 的原因，是让调用者使用带项目
前缀的路径：

```cpp
#include <travel_cpp/get_input.h>
```

这样即使依赖项中也有 `log.h` 或 `error_code.h`，也不容易发生同名冲突。不要在
源码中使用 `../include/...` 这类相对路径；CMake 已经通过每个库目标的
`target_include_directories(... PUBLIC ...)` 向消费者传递了正确的包含根目录。

## 公开头和私有头

- 会被主程序、测试或其他库使用的 API 放在 `include/travel_cpp/`。
- 只有某个 `.cpp` 实现需要的私有头，可以放在 `src/` 或 `src/detail/`，不要将其目录
  设为 `PUBLIC` include 路径。
- 物理上将文件放入统一的 `include/` 和 `src/` 并不会取消模块边界；本项目仍使用
  `lib_log`、`error_code` 和 `get_input` 三个独立构建目标表达责任和依赖。

## 新增一个模块

1. 在 `include/travel_cpp/` 中添加公开头，在 `src/` 中添加对应实现。
2. 在顶层 `CMakeLists.txt` 中用 `add_library` 定义目标，并为它设置
   `target_include_directories(... PUBLIC "${TRAVEL_CPP_PUBLIC_INCLUDE_DIR}")`。
3. 用 `target_link_libraries` 声明依赖：公开 API 中暴露的依赖用 `PUBLIC`，仅 `.cpp`
   内部使用的依赖用 `PRIVATE`。
4. 在 `tests/` 增加单元测试，并将测试目标链接到新库。

## 构建与测试

目标系统需要预先安装 CMake 可发现的 GoogleTest。

```bash
bash build.sh
```

如果只需要构建主程序，可以关闭测试：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF
cmake --build build --parallel
```
