#!/bin/bash
# build.sh

echo "Building GUI Media Player..."

mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=Release ..

make -j$(nproc 2>/dev/null || echo 4)

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "Run with: ./run.sh"
else
    echo "❌ Build failed!"
    exit 1
fi

cd ..