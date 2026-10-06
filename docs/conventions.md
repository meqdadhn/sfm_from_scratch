# Conventions

Fill this in during M0, before writing projection code. Every later stage depends on it.

## Frames

- World frame: <!-- e.g. local ENU, right-handed, Z up -->
- Camera frame: <!-- e.g. X right, Y down, Z forward (OpenCV) -->

## Pose

- Stored as: <!-- R_cw + t, or R_cw + center C, or quaternion ... -->
- Mapping: <!-- x_cam = R (X - C)  /  x_cam = R X + t -->
- Relation between t and C: <!-- -->

## Image coordinates

- Pixel origin: <!-- top-left corner of the top-left pixel, or its center? -->
- Normalized coordinates: <!-- after undistortion, x = (u - cx) / fx ... -->

## Camera model

- Intrinsics: <!-- fx, fy, cx, cy -->
- Distortion: <!-- OpenCV k1, k2, p1, p2, k3 -->

## Rotation conventions

- Omega-phi-kappa: <!-- order and sign, matching the PhD code (see ../SfM) -->

## Units

- Angles: <!-- radians internally, degrees in config/logs -->
- Distances: <!-- meters once georeferenced; arbitrary before -->
