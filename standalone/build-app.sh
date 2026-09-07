#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
cmake -S . -B build-standalone -DCMAKE_BUILD_TYPE=Release -DDUBL_BUILD_PLUGIN=OFF
cmake --build build-standalone --target dubl_render --parallel
swift build --package-path standalone
app=build-standalone/DUBL.app
mkdir -p "$app/Contents/MacOS"
cp standalone/Info.plist "$app/Contents/Info.plist"
cp standalone/.build/debug/DUBL "$app/Contents/MacOS/DUBL"
cp build-standalone/dubl_render "$app/Contents/MacOS/dubl_render"
codesign --force --deep --sign - "$app"
codesign --verify --deep --strict "$app"
