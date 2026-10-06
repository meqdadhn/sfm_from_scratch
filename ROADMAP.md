# SfM from scratch — milestone roadmap

A personal rebuild of the incremental SfM pipeline, written from scratch.

- **Reference implementation:** `../sfm_incremental` (compare results against it, don't copy it)
- **Original PhD code:** `../SfM`

## Ground rules

1. **Get it working end to end first, then improve each stage.** Get a crude but complete pipeline working early
   (M0–M6 with OpenCV shortcuts allowed), then swap in your own implementations one at a time.
   You should always have a working system with a baseline to compare against.
2. **Write it yourself first, then read the reference.** For each milestone, write your version, make it pass
   your own tests, and only then diff it against `sfm_incremental`. Write down every difference that
   surprised you in `NOTES.md`.
3. **Synthetic data before real data.** Every geometric stage gets a synthetic test with known ground truth
   before you run it on images.
4. **Make different design choices on purpose.** Choose different designs where it's interesting (see
   "Deliberate deviations" below), so the project reflects your own thinking.

## Datasets

| Purpose | Dataset |
|---|---|
| Unit tests | Synthetic scenes (your own generator, M0) |
| First real run | Small ordered sequence, 20–50 images (e.g. a COLMAP sample, or `sfm_incremental` test data) |
| Aerial / your domain | A drone dataset with EXIF GPS (the one `tools/preprocess` handles) |
| Scale test | 200+ images |

For every real dataset, run `sfm_incremental` once and keep its `poses.txt` as the reference output.

---

## Milestone 0 — Project skeleton and synthetic world

**Build**
- CMake project with Eigen, OpenCV, Ceres, glog, GoogleTest
- Core types: `Camera` (pinhole + OpenCV distortion), `Pose`, `Image`, `Point3D`, `Observation`
- Synthetic scene generator: random 3D points, cameras on a trajectory (circle, forward motion, aerial grid),
  projection with pixel noise, optional outlier injection
- Choose and **write down** your conventions: world→camera vs. camera→world, R/t vs. R/C, pixel origin

**Done when**
- [ ] `project(camera, pose, X)` round-trips with `unproject` to 1e-9
- [ ] The generator produces noise-free observations that reproject with 0 error
- [ ] Conventions are documented in `docs/conventions.md`

**Concepts:** coordinate frames, distortion model, why conventions cause most SfM bugs.

---

## Milestone 1 — Toy bundle adjustment (synthetic only)

This is the key milestone. Spend as much time on it as you need.

**Build**
- Reprojection residual as a templated functor, first undistorted pinhole, then with distortion
- `ceres::Problem` with parameter blocks for poses, points, and (shared) intrinsics
- Gauge fixing: handle the 7-DoF similarity ambiguity
- Robust loss option (none / Huber / Cauchy)

**Experiments** (write results in `NOTES.md`)
- [ ] Remove the gauge fixing. What does the solver report? What happens to the solution?
- [ ] Fix one camera only. Which DoF is still free? Fix it with a `SubsetManifold` on one center coordinate.
- [ ] Inject 5% outliers: compare no loss vs. Huber vs. Cauchy (final RMS, pose error)
- [ ] `DENSE_SCHUR` vs `SPARSE_SCHUR` vs `DENSE_QR`: timing as the number of cameras grows
- [ ] Write the **analytic Jacobian** for the pinhole residual; verify with `ceres::GradientChecker`
- [ ] Compare angle-axis + translation vs. angle-axis + center vs. quaternion + `EigenQuaternionManifold`

**Done when**
- [ ] BA recovers ground truth from perturbed poses/points (rotation < 0.01°, position < 1e-4 × scene size)
- [ ] Uses the modern Ceres API (`SetManifold`, not the removed `SetParameterization`)

**Concepts:** nonlinear least squares, LM, autodiff via Jets, manifolds, Schur complement, gauge freedom.

**Read afterwards:** `sfm_incremental/src/cost_functions.h`, `src/bundle_adjustment.cpp`.

---

## Milestone 2 — Features and matching

**Build**
- Feature extraction: start with `cv::SIFT`, add RootSIFT
- Keypoint undistortion to normalized coordinates
- Matching: FLANN kNN + ratio test + optional cross-check
- Pair selection: exhaustive, sequential (window N)
- Feature cache on disk (invalidated when params change)

**Done when**
- [ ] Matches between two overlapping real images look right in a visualization (draw them)
- [ ] Second run loads features from cache

**Optional deep dive (later):** implement your own SIFT (scale space, DoG extrema, orientation, descriptor)
and compare against OpenCV on a homography test, as `sfm_incremental/tests/test_features.cpp` does.

---

## Milestone 3 — Two-view geometry (relative orientation)

**Build**
- 5-point essential matrix in RANSAC (start with `cv::findEssentialMat`, later write your own RANSAC loop)
- Decompose E → 4 solutions → cheirality check
- Nonlinear refinement of the relative pose (Sampson error, Ceres)
- Homography check (H/E inlier ratio) and median triangulation angle per pair
- View graph: nodes = images, edges = verified pairs with relative pose + inliers

**Done when**
- [ ] Synthetic: recovers relative rotation < 0.1° for general motion **and** pure forward motion
- [ ] Planar / pure-rotation scenes are detected via the H/E ratio
- [ ] View graph is cached and reloadable

**Concepts:** epipolar geometry, E vs. F, degeneracies, Sampson approximation, RANSAC confidence math.

---

## Milestone 4 — Triangulation

**Build**
- Linear (DLT) two-view and multi-view triangulation
- Robust multi-view triangulation (drop the worst observation, re-triangulate)
- Filters: cheirality, reprojection error, minimum triangulation angle

**Done when**
- [ ] Synthetic: noise-free → exact; one corrupted observation is rejected
- [ ] Points with near-zero baseline are rejected

---

## Milestone 5 — Image registration

`sfm_incremental` uses **your PhD method**: rotation from rotation averaging over the ROPs to registered
neighbours, then position with R fixed. Implement both that and a PnP baseline, and compare them.

**Build**
- Single rotation averaging: candidate rotations from each registered neighbour's ROP, RANSAC,
  Horn's quaternion mean (verify it equals the chordal L2 mean)
- Position with fixed rotation: linear solve from 2D–3D correspondences + least trimmed squares
- Pose refinement (small Ceres problem, robust loss)
- Single-neighbour fallback
- **Baseline:** `cv::solvePnPRansac` (P3P / EPnP)

**Done when**
- [ ] Synthetic: recovers pose with outliers present
- [ ] Two disagreeing rotation candidates → registration fails cleanly
- [ ] Comparison table in `NOTES.md`: your method vs. PnP (accuracy, robustness, speed)

---

## Milestone 6 — Incremental mapper (first end-to-end run)

**Build**
- Seed pair selection (strongest connections, triangulation angle ≥ threshold, low H/E ratio)
- Registration loop: pick next image → register → triangulate new points → BA → filter
- Local (window) BA every N images, global BA every M images
- Retry for previously failed images
- Export: `poses.txt`, `points.ply`

**Done when**
- [ ] Synthetic sequence reconstructs every image; poses match ground truth after a similarity alignment
- [ ] **First real dataset reconstructs**: inspect the point cloud in MeshLab/CloudCompare
- [ ] `tools/evaluate_poses.py` (from the reference repo) vs. `sfm_incremental` output: report
      rotation/position error after alignment

🎯 **At this point you have a working SfM pipeline.** Everything after this improves it.

---

## Milestone 7 — Tracks and final BA

**Build**
- Union-find track building over all registered images
- Track triangulation + several rounds of full BA with point filtering
- Optional intrinsics refinement (focal, principal point, distortion, each toggleable)

**Done when**
- [ ] Final reprojection RMS < 1 px on the real dataset
- [ ] Mean track length and point count reported and compared to the reference

---

## Milestone 8 — Priors and georeferencing (your domain)

**Build**
- Python preprocessing: EXIF/GPS/XMP → camera groups → local ENU → config
- Trajectory-based pair selection (kNN / radius)
- GNSS position priors as BA residuals (weighted), or a similarity alignment to GPS afterwards; try both
- Omega-phi-kappa export, matching your PhD convention, with a unit test against the original rotation matrix

**Done when**
- [ ] Aerial dataset reconstructs in metric, georeferenced coordinates
- [ ] Trajectory pair selection reduces matching time vs. exhaustive with no loss in registered images

---

## Milestone 9 — Orthophoto

**Build**
- DEM from sparse points (IDW grid, median filter, hull fill)
- Ortho rendering: per pixel → Z from DEM → project into the best camera → bilinear sample

**Done when**
- [ ] Synthetic planar ground test matches the texture
- [ ] Real aerial ortho looks right when overlaid on a map

---

## Milestone 10 — Robustness, performance, polish

- Caching for every expensive stage (features, view graph, incremental state)
- OpenMP where it pays off; profile first
- YAML config, logging, clean CLI
- README with results (images, error tables, timings), pipeline diagram

---

## Deliberate deviations (pick at least two)

- [ ] Quaternion pose parameterization instead of angle-axis
- [ ] Analytic Jacobians instead of autodiff
- [ ] Your own RANSAC (+ LO-RANSAC or MAGSAC-style scoring) instead of OpenCV's
- [ ] Own SIFT implementation
- [ ] **Global SfM** variant: rotation averaging over the whole view graph + translation averaging,
      then one BA, compared with incremental
- [ ] GNSS priors inside BA vs. post-hoc alignment
- [ ] Learned features (SuperPoint/LightGlue via ONNX) as an alternative frontend

## Suggested reading

- Hartley & Zisserman, *Multiple View Geometry*: ch. 9–12 (epipolar geometry, triangulation), ch. 18 (BA)
- Triggs et al., *Bundle Adjustment — A Modern Synthesis* (2000)
- Schönberger & Frahm, *Structure-from-Motion Revisited* (COLMAP, CVPR 2016)
- Ceres Solver tutorial: "Bundle Adjustment" and "Modeling Non-linear Least Squares" (manifolds, loss functions)
- Nistér, *An Efficient Solution to the Five-Point Relative Pose Problem* (2004)
- Hartley et al., *Rotation Averaging* (IJCV 2013)

## Progress log

Estimates assume solid C++ and photogrammetry background, new to Ceres. They include debugging time.
Core total: ~165–275 h (M0–M6 alone, ~110–180 h, is already a complete project).

| Milestone | Est. hours | Actual hours | Started | Done | Notes |
|---|---|---|---|---|---|
| M0 Skeleton | 8–15 | | | | |
| M1 Toy BA | 25–40 | | | | |
| M2 Features | 8–15 | | | | |
| M3 Two-view | 15–25 | | | | |
| M4 Triangulation | 6–10 | | | | |
| M5 Registration | 15–25 | | | | |
| M6 Mapper | 30–50 | | | | |
| M7 Tracks + BA | 10–15 | | | | |
| M8 Priors | 20–30 | | | | |
| M9 Ortho | 12–20 | | | | |
| M10 Polish | 15–30 | | | | |
| **Core total** | **165–275** | | | | |

### Optional extras

| Extra | Est. hours | Actual hours | Done |
|---|---|---|---|
| Quaternion parameterization | 3–5 | | |
| Own RANSAC (+ LO-RANSAC) | 10–15 | | |
| Own 5-point solver (Nistér) | 15–25 | | |
| Own SIFT | 30–50 | | |
| Global SfM variant | 30–50 | | |
| Learned features (ONNX) | 15–25 | | |
