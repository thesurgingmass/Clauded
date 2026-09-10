#!/bin/bash
# Builds OctaClone in release mode and wraps it into a double-clickable
# OctaClone.app, native to Apple Silicon (arm64). Run this on a Mac with
# Xcode or the Xcode Command Line Tools installed (`xcode-select --install`).
set -euo pipefail

cd "$(dirname "$0")/.."

APP_NAME="OctaClone"
BUILD_DIR=".build/release"
APP_BUNDLE="${APP_NAME}.app"

echo "==> Building ${APP_NAME} (release, arm64)…"
swift build -c release --arch arm64

echo "==> Assembling ${APP_BUNDLE}…"
rm -rf "${APP_BUNDLE}"
mkdir -p "${APP_BUNDLE}/Contents/MacOS"
mkdir -p "${APP_BUNDLE}/Contents/Resources"

cp "${BUILD_DIR}/${APP_NAME}" "${APP_BUNDLE}/Contents/MacOS/${APP_NAME}"
cp "Resources/Info.plist" "${APP_BUNDLE}/Contents/Info.plist"

echo "==> Ad-hoc code signing (so Gatekeeper doesn't refuse a local, unsigned build)…"
codesign --force --deep --sign - "${APP_BUNDLE}"

echo "==> Done: ${APP_BUNDLE}"
echo "    Run it with: open \"${APP_BUNDLE}\""
echo "    If downloaded/copied from elsewhere, macOS may still quarantine it;"
echo "    if so, clear that with: xattr -dr com.apple.quarantine \"${APP_BUNDLE}\""
