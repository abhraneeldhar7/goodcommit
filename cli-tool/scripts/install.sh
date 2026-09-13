#!/bin/bash

# scripts/install.sh - Curl installer for Linux/macOS
#
# This script downloads and installs the xommit binary.
# It detects your OS and architecture, downloads the right binary,
# and installs it to /usr/local/bin.
#
# Usage:
#   curl -fsSL https://raw.githubusercontent.com/abhraneeldhar7/xommit/master/cli-tool/scripts/install.sh | bash
#
# What it does:
#   1. Detects your OS (Linux/macOS) and architecture (x64/arm64)
#   2. Downloads the binary from GitHub Releases
#   3. Makes it executable
#   4. Moves it to /usr/local/bin (may ask for sudo)
#   5. Prints next steps
#
# Requirements:
#   - curl or wget
#   - Internet connection

set -e

if ! command -v git > /dev/null 2>&1; then
  echo "Error: git is not installed."
  echo "Install it from: https://git-scm.com/install/"
  exit 1
fi

REPO="https://github.com/abhraneeldhar7/xommit/releases/latest/download"
INSTALL_DIR="/usr/local/bin"

# Detect operating system
detect_os() {
  OS=$(uname -s | tr '[:upper:]' '[:lower:]')
  case "$OS" in
    linux*)  echo "linux" ;;
    darwin*) echo "macos" ;;
    msys*|mingw*|cygwin*)
      echo "windows"
      ;;
    *)       echo "unsupported" ;;
  esac
}

# Detect CPU architecture
detect_arch() {
  ARCH=$(uname -m)
  case "$ARCH" in
    x86_64|amd64) echo "x64" ;;
    aarch64|arm64) echo "arm64" ;;
    *) echo "unsupported" ;;
  esac
}

OS=$(detect_os)
ARCH=$(detect_arch)

# Check for unsupported OS
if [ "$OS" = "unsupported" ]; then
  echo "Error: Unsupported operating system: $(uname -s)"
  echo "For Windows, download xommit.exe manually from:"
  echo "  https://github.com/abhraneeldhar7/xommit/releases"
  exit 1
fi

# Check for unsupported architecture
if [ "$ARCH" = "unsupported" ]; then
  echo "Error: Unsupported architecture: $(uname -m)"
  exit 1
fi

# Windows users should use npm or manual download
if [ "$OS" = "windows" ]; then
  echo "Error: Windows is not supported by this installer."
  echo "Use one of these methods instead:"
  echo ""
  echo "  1. npm (requires Node.js):"
  echo "     npm install -g xommit"
  echo ""
  echo "  2. Manual download:"
  echo "     Download xommit.exe from:"
  echo "     https://github.com/abhraneeldhar7/xommit/releases"
  echo "     Place it in a directory in your PATH."
  exit 1
fi

BINARY_NAME=""
case "$OS" in
  linux) BINARY_NAME="xommit" ;;
  macos) BINARY_NAME="xommit-macos" ;;
esac
DOWNLOAD_URL="${REPO}/${BINARY_NAME}"

echo "Detected: ${OS} ${ARCH}"
echo "Downloading from: ${DOWNLOAD_URL}"
echo ""

# Download the binary
if command -v curl > /dev/null 2>&1; then
  curl -fsSL "$DOWNLOAD_URL" -o /tmp/xommit
elif command -v wget > /dev/null 2>&1; then
  wget -q "$DOWNLOAD_URL" -O /tmp/xommit
else
  echo "Error: Neither curl nor wget found."
  echo "Install curl or wget and try again."
  exit 1
fi

# Make it executable
chmod +x /tmp/xommit

# Install to /usr/local/bin
if [ -w "$INSTALL_DIR" ]; then
  mv /tmp/xommit "${INSTALL_DIR}/xommit"
else
  echo "Installing to ${INSTALL_DIR} (may require sudo)..."
  sudo mv /tmp/xommit "${INSTALL_DIR}/xommit"
fi

echo ""
echo "xommit installed successfully."
echo ""
echo "Next steps:"
echo "  1. Set your API key:  xommit --connect"
echo "  2. Stage some files:  git add <files>"
echo "  3. Run:               xommit \"your vague message\""
