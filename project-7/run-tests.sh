#!/bin/bash

# automated test script for Mason's visual effects features
# runs all test cases with different parameter combinations

echo "======================================"
echo "Bread Final - Visual Effects Tests"
echo "======================================"
echo ""

# find qt bin directory for windows
if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "win32" ]]; then
  export PATH="/c/Qt/6.9.2/mingw_64/bin:$PATH"
fi

# find build directory
BUILD_PROJECT_DIR=$(find build -type d -name "*-Release" -print -quit)

if [ -z "$BUILD_PROJECT_DIR" ]; then
  echo "warning: release build not found. looking for debug build..."
  BUILD_PROJECT_DIR=$(find build -type d -name "*-Debug" -print -quit)
fi

if [ -z "$BUILD_PROJECT_DIR" ]; then
  echo "error: no build directory found in ./build/"
  echo "please build the project first with: cd build && cmake .. && make -j4"
  exit 1
fi

EXECUTABLE_PATH="$BUILD_PROJECT_DIR/BreadFinal"

# check if executable exists
if [ ! -x "$EXECUTABLE_PATH" ] && [ ! -f "$EXECUTABLE_PATH.exe" ]; then
  EXECUTABLE_PATH="$BUILD_PROJECT_DIR/bread-final"
  if [ ! -x "$EXECUTABLE_PATH" ] && [ ! -f "$EXECUTABLE_PATH.exe" ]; then
    echo "error: executable not found at $BUILD_PROJECT_DIR"
    echo "please build the project first"
    exit 1
  fi
fi

echo "found executable: $EXECUTABLE_PATH"

# create output directory
mkdir -p student_outputs/final_project

echo "starting automated tests..."
echo ""

# test counter
test_num=0

# ========================================
# fog tests
# ========================================

test_num=$((test_num + 1))
echo "[$test_num] fog - disabled (baseline)"
"$EXECUTABLE_PATH" scenefiles/test_fog.json \
  --disable-fog --disable-normal-mapping --disable-scrolling --disable-instancing \
  --headless -o student_outputs/final_project/fog_disabled.png

test_num=$((test_num + 1))
echo "[$test_num] fog - enabled (default settings)"
"$EXECUTABLE_PATH" scenefiles/test_fog.json \
  --enable-fog --fog-start 8.0 --fog-end 20.0 --fog-color 0.8,0.85,0.9 \
  --disable-normal-mapping --disable-scrolling --disable-instancing \
  --headless -o student_outputs/final_project/fog_enabled.png

test_num=$((test_num + 1))
echo "[$test_num] fog - near start distance (5.0)"
"$EXECUTABLE_PATH" scenefiles/test_fog.json \
  --enable-fog --fog-start 5.0 --fog-end 20.0 --fog-color 0.8,0.85,0.9 \
  --disable-normal-mapping --disable-scrolling --disable-instancing \
  --headless -o student_outputs/final_project/fog_near_start.png

test_num=$((test_num + 1))
echo "[$test_num] fog - far start distance (15.0)"
"$EXECUTABLE_PATH" scenefiles/test_fog.json \
  --enable-fog --fog-start 15.0 --fog-end 25.0 --fog-color 0.8,0.85,0.9 \
  --disable-normal-mapping --disable-scrolling --disable-instancing \
  --headless -o student_outputs/final_project/fog_far_start.png

test_num=$((test_num + 1))
echo "[$test_num] fog - warm color"
"$EXECUTABLE_PATH" scenefiles/test_fog.json \
  --enable-fog --fog-start 8.0 --fog-end 20.0 --fog-color 1.0,0.9,0.7 \
  --disable-normal-mapping --disable-scrolling --disable-instancing \
  --headless -o student_outputs/final_project/fog_color_warm.png

# ========================================
# normal mapping tests
# ========================================

test_num=$((test_num + 1))
echo "[$test_num] normal mapping - disabled (baseline)"
"$EXECUTABLE_PATH" scenefiles/test_normal_mapping.json \
  --disable-fog --disable-normal-mapping --disable-scrolling --disable-instancing \
  --headless -o student_outputs/final_project/normal_map_disabled.png

test_num=$((test_num + 1))
echo "[$test_num] normal mapping - enabled"
"$EXECUTABLE_PATH" scenefiles/test_normal_mapping.json \
  --disable-fog --enable-normal-mapping --disable-scrolling --disable-instancing \
  --headless -o student_outputs/final_project/normal_map_enabled.png

# ========================================
# scrolling texture tests
# ========================================

test_num=$((test_num + 1))
echo "[$test_num] scrolling - disabled (baseline)"
"$EXECUTABLE_PATH" scenefiles/test_scrolling.json \
  --disable-fog --disable-normal-mapping --disable-scrolling --disable-instancing \
  --headless -o student_outputs/final_project/scrolling_disabled.png

test_num=$((test_num + 1))
echo "[$test_num] scrolling - enabled (medium speed, horizontal)"
"$EXECUTABLE_PATH" scenefiles/test_scrolling.json \
  --disable-fog --disable-normal-mapping --enable-scrolling \
  --scroll-speed 0.5 --scroll-direction 1.0,0.0 --disable-instancing \
  --headless -o student_outputs/final_project/scrolling_enabled.png

test_num=$((test_num + 1))
echo "[$test_num] scrolling - fast speed (2.0)"
"$EXECUTABLE_PATH" scenefiles/test_scrolling.json \
  --disable-fog --disable-normal-mapping --enable-scrolling \
  --scroll-speed 2.0 --scroll-direction 1.0,0.0 --disable-instancing \
  --headless -o student_outputs/final_project/scrolling_fast.png

test_num=$((test_num + 1))
echo "[$test_num] scrolling - vertical direction"
"$EXECUTABLE_PATH" scenefiles/test_scrolling.json \
  --disable-fog --disable-normal-mapping --enable-scrolling \
  --scroll-speed 0.5 --scroll-direction 0.0,1.0 --disable-instancing \
  --headless -o student_outputs/final_project/scrolling_vertical.png

# ========================================
# instancing tests
# ========================================

test_num=$((test_num + 1))
echo "[$test_num] instancing - disabled (baseline)"
"$EXECUTABLE_PATH" scenefiles/test_instancing.json \
  --disable-fog --disable-normal-mapping --disable-scrolling --disable-instancing \
  --headless -o student_outputs/final_project/instancing_disabled.png

test_num=$((test_num + 1))
echo "[$test_num] instancing - enabled (100 cubes)"
"$EXECUTABLE_PATH" scenefiles/test_instancing.json \
  --disable-fog --disable-normal-mapping --disable-scrolling --enable-instancing \
  --headless -o student_outputs/final_project/instancing_enabled_100.png

test_num=$((test_num + 1))
echo "[$test_num] instancing - random generation 1"
"$EXECUTABLE_PATH" scenefiles/test_instancing.json \
  --disable-fog --disable-normal-mapping --disable-scrolling --enable-instancing \
  --headless -o student_outputs/final_project/instancing_random_1.png

test_num=$((test_num + 1))
echo "[$test_num] instancing - random generation 2"
"$EXECUTABLE_PATH" scenefiles/test_instancing.json \
  --disable-fog --disable-normal-mapping --disable-scrolling --enable-instancing \
  --headless -o student_outputs/final_project/instancing_random_2.png

test_num=$((test_num + 1))
echo "[$test_num] instancing - random generation 3"
"$EXECUTABLE_PATH" scenefiles/test_instancing.json \
  --disable-fog --disable-normal-mapping --disable-scrolling --enable-instancing \
  --headless -o student_outputs/final_project/instancing_random_3.png

# ========================================
# combined features test
# ========================================

test_num=$((test_num + 1))
echo "[$test_num] all features - combined"
"$EXECUTABLE_PATH" scenefiles/test_all_features.json \
  --enable-fog --fog-start 8.0 --fog-end 20.0 \
  --enable-normal-mapping --enable-scrolling --scroll-speed 0.5 \
  --enable-instancing \
  --headless -o student_outputs/final_project/all_features_combined.png

echo ""
echo "======================================"
echo "all tests complete!"
echo "======================================"
echo "outputs saved to: student_outputs/final_project/"
echo ""
echo "total tests run: $test_num"
echo ""
echo "next steps:"
echo "1. run unit tests: cd build && ctest -V"
echo "2. view screenshots in student_outputs/final_project/"
echo ""
echo "note: for different instance counts (50, 100, 200),"
echo "modify realtime.cpp generateInstances() and rebuild"
