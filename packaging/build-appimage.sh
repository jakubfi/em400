#!/usr/bin/env bash
#
# Build an AppImage of em400.
#
# Assumes em400 has already been configured and built in $BUILD_DIR with the
# Qt6 UI enabled. Stages a `cmake --install` into an AppDir, adds the
# freedesktop .desktop file and icon, then runs linuxdeploy (+ qt plugin) to
# bundle the shared libraries and produce a single-file AppImage.
#
# Environment overrides:
#   BUILD_DIR - CMake build directory (default: build)
#   APPDIR - AppDir to assemble (default: AppDir)
#   OUTDIR - where the .AppImage is written (default: dist)
#   ARCH - target architecture (default: x86_64)
#
# Requires: cmake, curl, and a FUSE-capable environment
# (or set APPIMAGE_EXTRACT_AND_RUN=1, which this script does by default for CI/Docker).

set -euo pipefail

# --- locate the repo root (this script lives in packaging/) -----------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build}"
APPDIR="${APPDIR:-$REPO_ROOT/AppDir}"
OUTDIR="${OUTDIR:-$REPO_ROOT/dist}"
ARCH="${ARCH:-x86_64}"

# FUSE is often unavailable in containers, let the tools extract-and-run
export APPIMAGE_EXTRACT_AND_RUN="${APPIMAGE_EXTRACT_AND_RUN:-1}"

# Pinned tooling (continuous builds - bump deliberately)
LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${ARCH}.AppImage"
LINUXDEPLOY_QT_URL="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-${ARCH}.AppImage"
TOOLS_DIR="${TOOLS_DIR:-$REPO_ROOT/.appimage-tools}"

# --- version string ---------------------------------------------------------

# Mirror cmake/appversion.cmake: prefer git tags, fall back to the appversion file
version() {
	local v
	if v="$(git -C "$REPO_ROOT" describe --tags --match 'release-*' 2>/dev/null)"; then
		echo "${v#release-}"
	else
		v="$(cat "$REPO_ROOT/appversion")"
		echo "${v#release-}"
	fi
}
VERSION="$(version)"

echo "==> Building em400 AppImage (version: $VERSION, arch: $ARCH)"

# --- sanity check the build -------------------------------------------------

if [ ! -x "$BUILD_DIR/em400" ]; then
	echo "error: $BUILD_DIR/em400 not found or not executable." >&2
	echo "       Configure & build first, e.g.:" >&2
	echo "       cmake -B '$BUILD_DIR' -DCMAKE_BUILD_TYPE=Release && cmake --build '$BUILD_DIR'" >&2
	exit 1
fi

# --- assemble the AppDir ----------------------------------------------------

rm -rf "$APPDIR"
cmake --install "$BUILD_DIR" --prefix "$APPDIR/usr"

# .desktop + icon (neither is installed by CMake)
install -Dm644 "$SCRIPT_DIR/em400.desktop" "$APPDIR/usr/share/applications/em400.desktop"
install -Dm644 "$REPO_ROOT/src/ui/qt6/icons/em400/em400_256.png" "$APPDIR/usr/share/icons/hicolor/256x256/apps/em400.png"

# QtMultimedia (panel QSoundEffect clicks) loads its backend as a plugin that
# linuxdeploy-plugin-qt does not pull from the linked lib. Stage it into the
# AppDir so linuxdeploy resolves the backend's libraries (FFmpeg) as deps.
QT_PLUGIN_DIR="$(${QMAKE:-qmake} -query QT_INSTALL_PLUGINS)"
if [ -d "$QT_PLUGIN_DIR/multimedia" ]; then
	mkdir -p "$APPDIR/usr/plugins/multimedia"
	cp -a "$QT_PLUGIN_DIR/multimedia/." "$APPDIR/usr/plugins/multimedia/"
fi

# --- fetch tooling ----------------------------------------------------------

mkdir -p "$TOOLS_DIR"
fetch() {
	local url="$1" dest="$2"
	if [ ! -f "$dest" ]; then
		echo "==> Fetching $(basename "$dest")"
		curl -fsSL "$url" -o "$dest"
		chmod +x "$dest"
	fi
}

fetch "$LINUXDEPLOY_URL" "$TOOLS_DIR/linuxdeploy.AppImage"
fetch "$LINUXDEPLOY_QT_URL" "$TOOLS_DIR/linuxdeploy-plugin-qt.AppImage"

# --- run linuxdeploy --------------------------------------------------------

# Do NOT bundle the host's audio libs: miniaudio dlopen()s ALSA/Pulse/JACK at
# runtime, so the AppImage should use whatever the target system provides
export LINUXDEPLOY_OUTPUT_VERSION="$VERSION"
export PATH="$TOOLS_DIR:$PATH"

rm -rf "$OUTDIR"
mkdir -p "$OUTDIR"
cd "$OUTDIR"

"$TOOLS_DIR/linuxdeploy.AppImage" \
	--appdir "$APPDIR" \
	--executable "$APPDIR/usr/bin/em400" \
	--desktop-file "$APPDIR/usr/share/applications/em400.desktop" \
	--icon-file "$APPDIR/usr/share/icons/hicolor/256x256/apps/em400.png" \
	--exclude-library "libasound.so*" \
	--exclude-library "libpulse*.so*" \
	--exclude-library "libjack.so*" \
	--plugin qt \
	--output appimage

# linuxdeploy names the file from the .desktop name + version + arch
# Normalise to a predictable em400-<version>-<arch>.AppImage
produced="$(ls -1 em400*.AppImage | head -n1)"
final="em400-${VERSION}-${ARCH}.AppImage"
if [ "$produced" != "$final" ]; then
	mv -f "$produced" "$final"
fi

echo "==> Done: $OUTDIR/$final"
