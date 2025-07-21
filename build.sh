#!/bin/bash
set -e

# === Clean previous builds ===
rm -rf dist build package
rm -f truss-*-linux.tar.gz
mkdir -p dist

# Compile C program
echo "🔧 Compiling truss analysis engine..."
gcc src/truss.c -o dist/truss_engine -lm

# === Install application dependencies ===
python3 -m pip install --upgrade pip
pip install -r requirements.txt

# === Install build tool (Nuitka) ===
pip install nuitka

# === Build with Nuitka ===
python3 -m nuitka \
  --standalone \
  --assume-yes-for-downloads \
  --follow-imports \
  --enable-plugin=pyside6 \
  --include-qt-plugins=sensible,imageformats,qml \
  --include-data-dir=src/assets=assets \
  --include-data-file=version.py=version.py \
  --include-data-file=dist/truss_engine=truss_engine \
  --output-dir=dist \
  src/truss.py


# === Make executable permissions ===
chmod +x dist/truss.dist/truss.bin

# === Get version ===
VERSION=$(python3 -c "from version import __version__; print(__version__)")

# === Prepare package directory ===
echo "Preparing package..."
mkdir -p package/truss-$VERSION
cp -r dist/truss.dist/* package/truss-$VERSION/
cp -r src/assets package/truss-$VERSION/ || true
cp version.py package/truss-$VERSION/ || true
cp -r installer package/truss-$VERSION/
cp LICENSE package/truss-$VERSION/
cp README.md package/truss-$VERSION/

# === Create tarball ===
echo "Creating distribution tarball..."
tar -czf Truss-linux.tar.gz -C package truss-$VERSION

echo
echo "✅ Linux build complete!"
echo "Created: Truss-linux.tar.gz"
echo "To install:"
echo "  1. tar -xzf Truss-linux.tar.gz"
echo "  2. cd truss-$VERSION"
echo "  3. sudo ./installer/install.sh"
