#!/bin/sh
# Builds aacdec.wasm: the decoder of the Fraunhofer FDK AAC Codec Library for
# Android, and nothing of its encoder, as a WebAssembly reactor module.
#
# Needs zstd, which compresses the module, and clang with a wasm32
# backend, wasm-ld, and the WASI sysroot plus the
# wasm32 compiler-rt builtins from https://github.com/WebAssembly/wasi-sdk
# (its "wasi-sysroot" and "libclang_rt" release assets are enough).
# fdk-aac's sources (v2.0.3) are the fdk-aac submodule:
#
#   git submodule update --init
#   WASI_SYSROOT=/path/to/wasi-sysroot RESOURCE_DIR=/path/to/resource-dir ./build.sh
#
# RESOURCE_DIR holds lib/wasm32-unknown-wasip1/libclang_rt.builtins.a and an
# include symlink to the compiler's own headers.
#
# fdk-aac is C++ that needs no C++ library: it is built without one, without
# exceptions or RTTI. Its sources are compiled unmodified, as its own
# CMakeLists.txt lists them for the decoder.
set -eu

: "${CLANG:=clang-22}"
: "${WASI_SYSROOT:?set WASI_SYSROOT}"
: "${RESOURCE_DIR:?set RESOURCE_DIR}"

here=$(cd "$(dirname "$0")" && pwd)
: "${FDK:=$here/fdk-aac}"
cd "$FDK"

libs="libAACdec libArithCoding libDRCdec libFDK libMpegTPDec libPCMutils libSACdec libSBRdec libSYS"
includes=""
for lib in $libs; do
	includes="$includes -I$lib/include"
done
# The .cpp files of the decoder's libraries, from CMakeLists.txt.
sources=$(awk '
	/^set\((AACDEC|ARITHCODING|DRCDEC|FDK|MPEGTPDEC|PCMUTILS|SACDEC|SBRDEC|SYS)_SRC/ { on = 1 }
	on {
		for (i = 1; i <= NF; i++) { f = $i; sub(/\)$/, "", f); if (f ~ /\.cpp$/) print f }
		if ($0 ~ /\)/) on = 0
	}
' CMakeLists.txt)

"$CLANG" --target=wasm32-wasip1 --sysroot="$WASI_SYSROOT" -resource-dir "$RESOURCE_DIR" \
	-O2 -msimd128 -mexec-model=reactor -Wl,--strip-all \
	-fno-exceptions -fno-rtti -nostdlib++ -x c++ -w \
	$includes $sources "$here/aacshim.cpp" \
	-o "$here/aacdec.wasm"

# komarugram downloads it zstd-compressed at the highest level, two
# fifths of its size:
# from the file, not a pipe, so that the frame tells its size and unpacking
# takes one buffer of it. zstd writes no name or time: the same module gives
# the same bytes.
zstd --ultra -22 -q -f "$here/aacdec.wasm" -o "$here/aacdec.wasm.zst"

echo "built $here/aacdec.wasm and $here/aacdec.wasm.zst"
