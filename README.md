# Convergence - Multiplayer sandbox game

Convergence is a multiplayer game for browsers and native platforms.

## Building

### Native executable

```bash
$ cmake -B build-native
$ cd build-native
$ make -j2
```

### Browser Wasm executable

Use Emscripten to output a WebAssembly build for browsers. It requires that you have [emsdk](https://github.com/emscripten-core/emsdk) installed and activated in your environment.

```bash
$ cmake -B build-emscripten -DCMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
$ cd build-emscripten
$ make -j2
```

