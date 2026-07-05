#!/usr/bin/env bash

# 在 RK3588/LubanCat 这类 Linux 目标机上构建并整理 demo 产物。
# 常用覆盖项：
#   BUILD_TYPE=Release ./build.sh
#   BUILD_TESTING=OFF ./build.sh
#   RUN_TESTS=0 ./build.sh
#   CLEAN=1 JOBS=4 ./build.sh
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

APP_NAME="gtest_demo"
TEST_APP_NAME="test_demo"

BUILD_TYPE="${BUILD_TYPE:-Debug}"
BUILD_TESTING="${BUILD_TESTING:-ON}"
RUN_TESTS="${RUN_TESTS:-1}"
CLEAN="${CLEAN:-0}"

BUILD_TYPE_LOWER="$(printf "%s" "${BUILD_TYPE}" | tr '[:upper:]' '[:lower:]')"
BUILD_DIR="${BUILD_DIR:-${SCRIPT_DIR}/build/${BUILD_TYPE_LOWER}}"
DEPLOY_DIR="${DEPLOY_DIR:-${SCRIPT_DIR}/deploy/rk3588-${BUILD_TYPE_LOWER}}"
JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)}"

echo "Project dir : ${SCRIPT_DIR}"
echo "Build type  : ${BUILD_TYPE}"
echo "Build tests : ${BUILD_TESTING}"
echo "Build dir   : ${BUILD_DIR}"
echo "Deploy dir  : ${DEPLOY_DIR}"
echo "Jobs        : ${JOBS}"
echo

if [[ "${CLEAN}" == "1" ]]; then
    echo "Cleaning build and deploy directories..."
    rm -rf "${BUILD_DIR}" "${DEPLOY_DIR}"
fi

echo "Configuring project..."
cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DBUILD_TESTING="${BUILD_TESTING}"

echo
echo "Building project..."
cmake --build "${BUILD_DIR}" --parallel "${JOBS}"

APP_PATH="${BUILD_DIR}/bin/${APP_NAME}"
TEST_APP_PATH="${BUILD_DIR}/bin/${TEST_APP_NAME}"

if [[ ! -x "${APP_PATH}" ]]; then
    echo "Error: executable not found: ${APP_PATH}" >&2
    echo "Please check APP_NAME in build.sh and add_executable(...) in CMakeLists.txt." >&2
    exit 1
fi

# 在构建目录中运行 CTest，确保能找到 CMake 生成的测试元数据。
if [[ "${BUILD_TESTING}" != "OFF" && "${RUN_TESTS}" == "1" ]]; then
    echo
    echo "Running unit tests..."
    cmake -E chdir "${BUILD_DIR}" ctest --output-on-failure
fi

echo
echo "Preparing deploy directory..."
mkdir -p "${DEPLOY_DIR}/bin"
cp "${APP_PATH}" "${DEPLOY_DIR}/bin/"

echo
echo "Run app:"
echo "  ${DEPLOY_DIR}/bin/${APP_NAME} --test_mode=demo"

if [[ "${BUILD_TESTING}" != "OFF" ]]; then
    echo
    echo "Run all unit tests:"
    echo "  cmake -E chdir ${BUILD_DIR} ctest --output-on-failure"

    echo
    echo "Run test binary directly:"
    echo "  ${TEST_APP_PATH}"

    echo
    echo "Debug one test case:"
    echo "  gdb --args ${TEST_APP_PATH} --gtest_filter=GetInputTest.ReadLongOptionEqualForm"
fi
