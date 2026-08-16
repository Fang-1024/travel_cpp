# 任何一步出错就立即停止，避免继续使用不完整的构建结果。
set -euo pipefail

# 得到项目根目录。这样无论从哪个目录运行本脚本，都能找到 CMakeLists.txt。
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 所有构建文件统一放在项目根目录下的 build/ 中。
# 最终程序位于 build/bin/，目录较浅，方便直接使用 GDB。
BUILD_DIR="${PROJECT_DIR}/build"

echo "1/3 配置项目（Debug 模式）"
cmake -S "${PROJECT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTING=ON

echo
echo "2/3 编译项目"
cmake --build "${BUILD_DIR}" --parallel

echo
echo "3/3 运行单元测试"
# CTest 需要在构建目录中运行，才能找到 CMake 生成的测试信息。
(
    cd "${BUILD_DIR}"
    ctest --output-on-failure
)

echo
echo "构建和测试完成。"
echo
echo "调试主程序："
echo "  gdb --args ${BUILD_DIR}/bin/gtest_demo --test_mode=demo"
echo
echo "调试测试程序："
echo "  gdb ${BUILD_DIR}/bin/test_demo"
echo
echo "只调试一个测试用例："
echo "  gdb --args ${BUILD_DIR}/bin/test_demo --gtest_filter=GetInputTest.ReadLongOptionEqualForm"
