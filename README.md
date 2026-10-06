# sfm_from_scratch

Incremental structure-from-motion, rebuilt from scratch as a personal project. See [ROADMAP.md](ROADMAP.md).

## Build

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Dependencies: Eigen 3.3+, OpenCV 4 (with contrib for SIFT before 4.4), Ceres (2.1+ for the Manifold API),
GoogleTest.

To use a Ceres installed in a custom prefix:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/opt/ceres-2.2
```
