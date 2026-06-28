#!/usr/bin/env bash

# ============================================================
# build.sh
#
# 用途：
#   在 RK3588 / LubanCat-5 这类 Linux 开发板上构建当前 C++ 工程。
#
# 默认行为：
#   1. 使用 Debug 模式构建，方便 gdb 调试
#   2. 使用独立 build 目录，不污染源码目录
#   3. 构建完成后，把可执行程序复制到 deploy 目录
#
# 常用命令：
#   ./build.sh
#   CLEAN=1 ./build.sh
#   BUILD_TYPE=Release ./build.sh
#   JOBS=4 ./build.sh
# ============================================================


# ------------------------------------------------------------
# 一、Shell 安全选项
# ------------------------------------------------------------
# -E:
#   让 ERR 错误处理在函数、子 shell 中也尽量生效。
#
# -e:
#   任意命令执行失败，脚本立刻退出。
#   这样可以避免 cmake 已经失败了，后面还继续 make。
#
# -u:
#   使用未定义变量时报错。
#   例如写错变量名时，可以尽早发现。
#
# -o pipefail:
#   管道命令中任意一步失败，整个管道就算失败。
#
# 初学者可以先把这一行理解为：
#   “脚本出错就停，不要带着错误继续执行。”
set -Eeuo pipefail


# ------------------------------------------------------------
# 二、定位脚本所在目录
# ------------------------------------------------------------
# ${BASH_SOURCE[0]}:
#   当前脚本文件本身的路径。
#
# dirname:
#   取出脚本所在目录。
#
# cd ... && pwd:
#   进入脚本目录后，打印绝对路径。
#
# 最终 SCRIPT_DIR 会变成项目根目录的绝对路径。
# 这样无论你在哪个目录执行 ./build.sh，脚本都能回到项目根目录工作。
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"


# 切换到项目根目录。
# 后面的 cmake、deploy、build 路径都基于这个目录。
cd "${SCRIPT_DIR}"


# ------------------------------------------------------------
# 三、项目基本配置
# ------------------------------------------------------------

# 当前 CMakeLists.txt 里生成的可执行程序名字是 gtest_demo。
# 如果你以后把 add_executable(gtest_demo ...) 改成别的名字，
# 这里也要同步修改。
APP_NAME="gtest_demo"


# BUILD_TYPE="${BUILD_TYPE:-Debug}" 的含义：
#
#   如果外部已经设置了 BUILD_TYPE，就使用外部的值；
#   如果外部没有设置 BUILD_TYPE，就默认使用 Debug。
#
# 例如：
#   ./build.sh
#       => BUILD_TYPE=Debug
#
#   BUILD_TYPE=Release ./build.sh
#       => BUILD_TYPE=Release
#
# Debug:
#   带调试信息，适合 gdb。
#
# Release:
#   编译优化更强，适合发布运行，但不适合初学阶段调试。
BUILD_TYPE="${BUILD_TYPE:-Debug}"

# 是否启用单元测试。
#
# ON:
#   CMake 会配置 tests/ 目录，并通过 FetchContent 获取 GoogleTest。
#
# OFF:
#   只构建主程序，不构建测试目标。
#
# RK3588 板端第一次启用 ON 时，需要能访问 GitHub，
# 因为 FetchContent 会下载 GoogleTest。
BUILD_TESTING="${BUILD_TESTING:-ON}"


# 是否在构建完成后立即运行 ctest。
#
# 1:
#   构建完成后运行所有单元测试。
#
# 0:
#   只构建，不运行测试。
#
# 调试某个测试时可以先设为 0，然后手动用 gdb 启动测试程序。
RUN_TESTS="${RUN_TESTS:-1}"


# 把 BUILD_TYPE 转成小写。
# 例如：
#   Debug   -> debug
#   Release -> release
#
# 这样目录名字会更统一：
#   build/debug
#   build/release
BUILD_TYPE_LOWER="$(printf "%s" "${BUILD_TYPE}" | tr '[:upper:]' '[:lower:]')"


# 构建目录。
#
# BUILD_DIR="${BUILD_DIR:-默认值}" 的含义和 BUILD_TYPE 类似：
#   外部指定了 BUILD_DIR，就用外部的；
#   没指定，就用默认路径。
#
# 默认路径示例：
#   build/debug
BUILD_DIR="${BUILD_DIR:-${SCRIPT_DIR}/build/${BUILD_TYPE_LOWER}}"


# 部署目录。
#
# 构建过程中的临时文件放在 build 目录；
# 最终准备运行、调试的产物放在 deploy 目录。
#
# 默认路径示例：
#   deploy/rk3588-debug
DEPLOY_DIR="${DEPLOY_DIR:-${SCRIPT_DIR}/deploy/rk3588-${BUILD_TYPE_LOWER}}"


# 并行编译数量。
#
# 如果外部指定 JOBS，就使用外部指定的值：
#   JOBS=4 ./build.sh
#
# 如果没指定，就用 getconf _NPROCESSORS_ONLN 获取 CPU 核心数量。
#
# 2>/dev/null:
#   把错误输出丢掉，避免 getconf 不可用时打印多余错误。
#
# || echo 4:
#   如果 getconf 失败，就默认使用 4。
JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)}"


# 是否清理旧构建。
#
# 默认不清理：
#   ./build.sh
#
# 清理后重新构建：
#   CLEAN=1 ./build.sh
CLEAN="${CLEAN:-0}"


# ------------------------------------------------------------
# 四、打印当前构建配置
# ------------------------------------------------------------
# 这一步不是必须的，但对初学者很有帮助。
# 你可以清楚看到脚本最终使用了哪些路径和配置。
echo "Project dir : ${SCRIPT_DIR}"
echo "Build type  : ${BUILD_TYPE}"
echo "Build dir   : ${BUILD_DIR}"
echo "Deploy dir  : ${DEPLOY_DIR}"
echo "Jobs        : ${JOBS}"
echo


# ------------------------------------------------------------
# 五、可选清理
# ------------------------------------------------------------
# [[ ... ]] 是 bash 里的条件判断写法。
#
# 如果 CLEAN=1，就删除当前构建目录和部署目录。
# 注意：这里只删除脚本自己管理的 build/debug 和 deploy/rk3588-debug，
# 不会删除源码。
if [[ "${CLEAN}" == "1" ]]; then
    echo "Cleaning build and deploy directories..."
    rm -rf "${BUILD_DIR}" "${DEPLOY_DIR}"
fi


# ------------------------------------------------------------
# 六、CMake 配置阶段
# ------------------------------------------------------------
# cmake -S:
#   指定源码目录。
#
# cmake -B:
#   指定构建目录。
#
# -DCMAKE_BUILD_TYPE:
#   指定构建类型。
#   Debug 模式通常会带 -g 调试信息，方便 gdb 调试。
#
# -DCMAKE_EXPORT_COMPILE_COMMANDS=ON:
#   生成 compile_commands.json。
#   这个文件对 VSCode、clangd、代码跳转、静态分析很有用。
echo "Configuring project..."
cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DBUILD_TESTING="${BUILD_TESTING}"


# ------------------------------------------------------------
# 七、编译阶段
# ------------------------------------------------------------
# 不建议直接写：
#   make -j$(nproc)
#
# 更推荐使用：
#   cmake --build
#
# 原因：
#   CMake 底层可能使用 Make，也可能使用 Ninja。
#   cmake --build 可以自动调用正确的构建工具。
echo
echo "Building project..."
cmake --build "${BUILD_DIR}" --parallel "${JOBS}"


# ------------------------------------------------------------
# 八、检查可执行文件是否生成
# ------------------------------------------------------------
# 当前项目的可执行文件默认在：
#   build/debug/gtest_demo
#
# -x:
#   判断文件是否存在，并且是否可执行。
#
# 如果没有找到，说明：
#   1. CMakeLists.txt 里的目标名可能改了；
#   2. APP_NAME 可能没有同步修改；
#   3. 构建过程可能有问题。
APP_PATH="${BUILD_DIR}/bin/${APP_NAME}"

if [[ ! -x "${APP_PATH}" ]]; then
    echo "Error: executable not found: ${APP_PATH}" >&2
    echo "Please check APP_NAME in build.sh and add_executable(...) in CMakeLists.txt." >&2
    exit 1
fi

# ------------------------------------------------------------
# 九、运行单元测试
# ------------------------------------------------------------
# ctest 是 CMake 自带的测试运行器。
#
# 这里用 cmake -E chdir 切换到构建目录再运行 ctest，
# 等价于：
#   cd build/debug
#   ctest --output-on-failure
#
# --output-on-failure:
#   只有测试失败时才打印详细输出，平时输出更干净。
if [[ "${BUILD_TESTING}" != "OFF" && "${RUN_TESTS}" == "1" ]]; then
    echo
    echo "Running unit tests..."
    cmake -E chdir "${BUILD_DIR}" ctest --output-on-failure
fi


# ------------------------------------------------------------
# 十、准备部署目录
# ------------------------------------------------------------
# mkdir -p:
#   如果目录不存在，就创建；
#   如果目录已经存在，也不会报错。
#
# 这里把最终可执行程序复制到：
#   deploy/rk3588-debug/bin/gtest_demo
#
# 这样 build 目录负责“构建过程”，deploy 目录负责“运行产物”。
echo
echo "Preparing deploy directory..."
mkdir -p "${DEPLOY_DIR}/bin"
cp "${APP_PATH}" "${DEPLOY_DIR}/bin/"


# ------------------------------------------------------------
# 十、打印使用提示
# ------------------------------------------------------------
echo
echo "Run app:"
echo "  ${DEPLOY_DIR}/bin/${APP_NAME} --test_mode=demo"

if [[ "${BUILD_TESTING}" != "OFF" ]]; then
    echo
    echo "Run all unit tests:"
    echo "  cmake -E chdir ${BUILD_DIR} ctest --output-on-failure"

    echo
    echo "Run get_input_test directly:"
    echo "  ${BUILD_DIR}/bin/get_input_test"

    echo
    echo "Debug one test case:"
    echo "  gdb --args ${BUILD_DIR}/bin/get_input_test --gtest_filter=GetInputTest.ReadLongOptionEqualForm"
fi