#!/usr/bin/env bash

set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

BUILD_TYPE="${BUILD_TYPE:-Debug}"
BUILD_DIR_ARG="${BUILD_DIR:-}"
INSTALL_DIR_ARG="${INSTALL_DIR:-}"
JOBS="${JOBS:-$(nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}"
CLEAN=0

usage() {
    cat <<EOF
Usage: ./build.sh [options]

Options:
  --debug              Build with debug symbols, no optimization. Default.
  --release            Build optimized release binary.
  --relwithdebinfo     Build optimized binary with debug symbols.
  --clean              Remove build and deploy directories before building.
  -j, --jobs N         Number of parallel build jobs. Default: CPU count.
  --build-dir DIR      Override build directory.
  --install-dir DIR    Override deploy/install directory.
  -h, --help           Show this help.

Examples:
  ./build.sh
  ./build.sh --clean
  ./build.sh --release -j 4
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --debug)
            BUILD_TYPE="Debug"
            ;;
        --release)
            BUILD_TYPE="Release"
            ;;
        --relwithdebinfo)
            BUILD_TYPE="RelWithDebInfo"
            ;;
        --clean)
            CLEAN=1
            ;;
        -j|--jobs)
            if [[ $# -lt 2 ]]; then
                echo "Missing value for $1" >&2
                exit 1
            fi
            JOBS="$2"
            shift
            ;;
        --build-dir)
            if [[ $# -lt 2 ]]; then
                echo "Missing value for $1" >&2
                exit 1
            fi
            BUILD_DIR_ARG="$2"
            shift
            ;;
        --install-dir)
            if [[ $# -lt 2 ]]; then
                echo "Missing value for $1" >&2
                exit 1
            fi
            INSTALL_DIR_ARG="$2"
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            usage
            exit 1
            ;;
    esac
    shift
done

case "$BUILD_TYPE" in
    Debug|Release|RelWithDebInfo|MinSizeRel)
        ;;
    *)
        echo "Unsupported BUILD_TYPE: $BUILD_TYPE" >&2
        exit 1
        ;;
esac

BUILD_DIR="${BUILD_DIR_ARG:-${PROJECT_ROOT}/build/${BUILD_TYPE}}"
INSTALL_DIR="${INSTALL_DIR_ARG:-${PROJECT_ROOT}/deploy/${BUILD_TYPE}}"

if ! [[ "$JOBS" =~ ^[0-9]+$ ]] || [[ "$JOBS" -lt 1 ]]; then
    echo "Invalid jobs value: $JOBS" >&2
    exit 1
fi

if ! command -v cmake >/dev/null 2>&1; then
    echo "cmake not found. Please install cmake first." >&2
    exit 1
fi

if [[ "$CLEAN" -eq 1 ]]; then
    case "$BUILD_DIR" in
        "$PROJECT_ROOT"/build/*)
            rm -rf "$BUILD_DIR"
            ;;
        *)
            echo "Refuse to clean build dir outside project build/: $BUILD_DIR" >&2
            exit 1
            ;;
    esac

    case "$INSTALL_DIR" in
        "$PROJECT_ROOT"/deploy/*)
            rm -rf "$INSTALL_DIR"
            ;;
        *)
            echo "Refuse to clean deploy dir outside project deploy/: $INSTALL_DIR" >&2
            exit 1
            ;;
    esac
fi

echo "Project root : $PROJECT_ROOT"
echo "Build type   : $BUILD_TYPE"
echo "Build dir    : $BUILD_DIR"
echo "Deploy dir   : $INSTALL_DIR"
echo "Jobs         : $JOBS"

cmake -S "$PROJECT_ROOT" \
      -B "$BUILD_DIR" \
      -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build "$BUILD_DIR" --parallel "$JOBS"
cmake --install "$BUILD_DIR" --prefix "$INSTALL_DIR"

echo
echo "Done."
echo "Binary: $INSTALL_DIR/bin/gtest_demo"
echo "Debug with: gdb $INSTALL_DIR/bin/gtest_demo"
