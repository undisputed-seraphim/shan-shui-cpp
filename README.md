
# C++/WebAssembly port

This directory contains a C++20 port of the original {Shan, Shui}* generator,
compiled to WebAssembly. The browser remains the rendering canvas: the WASM
module generates the same SVG output (byte-compatible up to last-ulp trig
differences), and a thin JS UI displays it.

## Layout

- `src/core/`  PRNG (Blum-Blum-Shub), Perlin noise, triangulation, utilities
- `src/draw/`  painter (op accumulator) + stroke/blob/div/texture
- `src/gen/`   tree (8 types), mount, arch, man, water generators
- `src/scene/` mount planner, chunk store, view assembly
- `src/api.cpp` wasm exports (embind): ss_init, ss_get_view, ss_reset, ss_noise
- `src/server/` pure-C++ HTTP static file server (no dependencies)
- `web/`       index.html + app.js UI
- `src/main_test.cpp` smoke test comparing against JS reference values

## Building

    cmake --preset host && cmake --build build-host     # ss_test + ss_server
    cmake --preset wasm && cmake --build build-wasm     # ss.js/ss.wasm + single-file variant

## Serving

    ./build-host/ss_server --port 8080 --root build-wasm
    # browse to http://<this-machine-ip>:8080

The `build-wasm/single/` directory contains the self-contained variant
(wasm embedded in ss.js) that also works from file://.

## Parity

Verified byte-identical to the original JavaScript against fixed seeds for all
generators (PRNG stream, perlin table, every SVG op). Known deviations: a
single 0.1px rounding difference in Arch.boat01 from V8-vs-libm trig ulps.
