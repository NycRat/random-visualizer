#!/bin/bash

# Variables
APP_NAME="RandomVisualizer"
APP_PATH="${APP_NAME}.app"
OUTPUT_DIR="/tmp/${APP_NAME}"
DMG_NAME="${APP_NAME}.dmg"
DMG_PATH="${OUTPUT_DIR}/${DMG_NAME}"
MOUNT_POINT="/Volumes/${APP_NAME}"

# -- CREATE APP

EXECUTABLE_NAME="RandomVisualizer"
EXECUTABLE_PATH="./build/desktop/random_visualizer"
SDL2_FRAMEWORK_PATH="$HOME/Downloads/SDL2.framework"
BUNDLE_DIR="${APP_NAME}.app"
INFO_PLIST="${BUNDLE_DIR}/Contents/Info.plist"

# build exe
make

# Create the bundle structure
mkdir -p "${BUNDLE_DIR}/Contents/MacOS"
mkdir -p "${BUNDLE_DIR}/Contents/Resources"
mkdir -p "${BUNDLE_DIR}/Contents/Frameworks"

# Copy icon
cp "${APP_NAME}.icns" "${BUNDLE_DIR}/Contents/Resources"

# Create Info.plist
cat <<EOF > "${INFO_PLIST}"
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>
    <string>${APP_NAME}</string>
    <key>CFBundleExecutable</key>
    <string>${EXECUTABLE_NAME}</string>
    <key>CFBundleIdentifier</key>
    <string>com.example.${APP_NAME}</string>
    <key>CFBundleIconFile</key>
    <string>${APP_NAME}.icns</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleSignature</key>
    <string>????</string>
</dict>
</plist>
EOF

# Copy the executable and SDL2 framework
cp "${EXECUTABLE_PATH}" "${BUNDLE_DIR}/Contents/MacOS/${EXECUTABLE_NAME}"
cp -R "${SDL2_FRAMEWORK_PATH}" "${BUNDLE_DIR}/Contents/Frameworks/"

# Set executable permissions
chmod +x "${BUNDLE_DIR}/Contents/MacOS/${EXECUTABLE_NAME}"

echo "Created ${BUNDLE_DIR} successfully."

# -- CREATE DMG

rm -rf "${OUTPUT_DIR}"
mkdir -p "${OUTPUT_DIR}"

# Create a blank disk image
hdiutil create -size 10m -fs HFS+ -volname "${APP_NAME}" "${DMG_PATH}"

# Mount the disk image
hdiutil attach "${DMG_PATH}" -mountpoint "${MOUNT_POINT}"

ln -s /Applications "${OUTPUT_DIR}/Applications"
cp -R "${OUTPUT_DIR}/Applications" "${MOUNT_POINT}"

# Copy the application into the disk image
cp -R "${APP_PATH}" "${MOUNT_POINT}"

# Unmount and finalize the disk image
hdiutil detach "${MOUNT_POINT}"
hdiutil convert "${DMG_PATH}" -format UDZO -o "./${APP_NAME}_final.dmg"
