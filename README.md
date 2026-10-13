# fdk-aac-wasm

`aacdec.wasm` is the decoder of the Fraunhofer FDK AAC Codec Library for
Android (fdk-aac), and nothing of its encoder, compiled to a WebAssembly
(WASI reactor) module. [komarugram](https://github.com/komarugif/komarugram-go)
downloads it to play AAC — voice messages and music in M4A — in a sandbox,
without FFmpeg.

## Contents

| File | What |
|---|---|
| `aacdec.wasm` | The module |
| `aacdec.wasm.zst` | The same module, zstd-compressed (`zstd --ultra -22`): what komarugram downloads |
| `aacshim.cpp` | The only surface the host can call: configure with an AudioSpecificConfig, decode raw access units, read back 16-bit samples |
| `build.sh` | How `aacdec.wasm` and `aacdec.wasm.zst` are built |
| `fdk-aac` | fdk-aac's sources, tag `v2.0.3`, as a git submodule of https://github.com/mstorsjo/fdk-aac, unmodified |

## Building

```sh
git clone --recurse-submodules --shallow-submodules https://github.com/komarugif/fdk-aac-wasm
cd fdk-aac-wasm
WASI_SYSROOT=/path/to/wasi-sysroot RESOURCE_DIR=/path/to/resource-dir ./build.sh
```

See the head of `build.sh` for the toolchain.

## License

The module contains fdk-aac v2.0.3, unmodified, under the Software License
for The Fraunhofer FDK AAC Codec Library for Android: see `NOTICE`, whose
complete text goes with every copy of the module. Its complete source code
is the `fdk-aac` submodule of this repository, free of charge. `aacshim.cpp`
and `build.sh` are Unlicense OR MIT.

That license grants no patent rights. AAC is covered by patents, licensed
through Via LA (https://www.via-la.com): using the module may need such a
license, in particular for commercial use. It is a separate module so that a
distribution can leave it out, or put another decoder in its place.

The module also links WASI libc from [wasi-sdk](https://github.com/WebAssembly/wasi-sdk)
(musl and wasi-libc's own code) and the compiler-rt builtins of LLVM; their
notices are in `licenses/wasi-libc`.
