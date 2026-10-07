# Conventions

Fixed in M0, before writing projection code. Every later stage depends on it.
Robotics conventions throughout (ROS REP-103 frames, `T_ab` pose notation, quaternions, ZYX roll-pitch-yaw).
Photogrammetric omega-phi-kappa appears only at the export boundary (M8).

Notation: `T_ab` / `R_ab` maps coordinates expressed in frame b into frame a (`x_a = R_ab x_b + t_ab`).
`T_ab` is also "the pose of frame b in frame a": `t_ab` is b's origin expressed in a.

## Frames

- World `w`: right-handed. Local ENU (X east, Y north, Z up) when georeferenced; arbitrary before.
- Camera `c` (optical frame): X right, Y down, Z forward (OpenCV / REP-103 `_optical`), origin at the
  projection center.
- Body `b` (only for human-readable angles): X forward, Y left, Z up (REP-103 FLU), same origin as `c`.
  `R_bc` is fixed:

  ```
  R_bc = [  0  0  1 ]     camera Z (forward) → body X
         [ -1  0  0 ]     camera X (right)   → body −Y
         [  0 -1  0 ]     camera Y (down)    → body −Z
  ```

## Pose

- Stored as `T_wc`: unit quaternion `q_wc` (camera → world rotation) and `t_wc` (camera position in
  world, a.k.a. the camera center C).
- Mapping world → camera: `x_c = R_wcᵀ (x_w − t_wc)`, i.e.

  ```
  T_cw = T_wc⁻¹ = [ R_wcᵀ   −R_wcᵀ t_wc ]
                  [ 0 0 0        1      ]
  ```

- Relation to OpenCV / BA-style `(R, t)`: those are `T_cw`, so `R_cw = R_wcᵀ`, `t_cw = −R_wcᵀ t_wc`,
  and `t_wc = −R_cwᵀ t_cw`.

## Image coordinates

- Pixel origin: (0, 0) is the **center** of the top-left pixel (OpenCV). u right, v down.
- Normalized coordinates: undistorted, on the z = 1 plane: `x_n = X_c / Z_c`, `y_n = Y_c / Z_c`.
- Projection: `x_n` → distortion → `x_d`, then `u = fx·x_d + cx`, `v = fy·y_d + cy`.

## Camera model

- Intrinsics: fx, fy, cx, cy (pixels), skew = 0.
- Distortion: OpenCV k1, k2, p1, p2, k3, applied to normalized coordinates.

## Rotation conventions

- Quaternions: Hamilton, unit norm, scalar `w ≥ 0` after normalization. Eigen's `Quaterniond(w, x, y, z)`
  constructor order; note Eigen stores `[x, y, z, w]` in memory (matters for raw `double*` in Ceres,
  where `EigenQuaternionManifold` expects that layout and `QuaternionManifold` expects `[w, x, y, z]`).
- Roll-pitch-yaw (logs, config, debugging): ZYX intrinsic,
  `R_wb = Rz(yaw) · Ry(pitch) · Rx(roll)`, for the FLU body frame in ENU. All zero = level, facing east;
  yaw counterclockwise from east. Camera orientation: `R_wc = R_wb · R_bc`.
- Drone metadata (aerospace, NED world / FRD body) uses the same ZYX formula but different frames;
  convert explicitly: `R_wb = R_ENU←NED · R_NED←FRD(rpy) · R_FRD←FLU`, with
  `R_ENU←NED = [[0,1,0],[1,0,0],[0,0,−1]]` and `R_FRD←FLU = diag(1,−1,−1)`.
- Omega-phi-kappa (export only, M8): `R_opk = Rx(ω) · Ry(φ) · Rz(κ)`, maps the photogrammetric camera
  frame (x right, y up, z back) → world. Matches `rotation_Mat` in `../SfM/SfM.cpp`.
  `R_opk = R_wc · diag(1, −1, −1)`.

## Units

- Angles: radians internally, degrees in config/logs.
- Distances: meters once georeferenced; arbitrary before.
