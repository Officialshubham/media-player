#!/bin/bash
# run.sh

# Build if needed
if [ ! -f "build/gui-player" ]; then
    echo "Building first..."
    ./build.sh
fi

# Set display
export DISPLAY=:0

echo "Starting GUI Media Player..."
./build/gui-player "$@"