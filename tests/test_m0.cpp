// M0 "done when" checks (see ROADMAP.md). Replace each GTEST_SKIP with a real test as you implement.

#include <gtest/gtest.h>

TEST(M0_Camera, ProjectUnprojectRoundTrip)
{
  GTEST_SKIP() << "TODO: project(camera, pose, X) then unproject, agree to 1e-9 (with distortion)";
}

TEST(M0_Pose, CenterAndTranslationAreConsistent)
{
  GTEST_SKIP() << "TODO: building a pose from (R, C) and from (R, t) gives the same projection";
}

TEST(M0_Synthetic, NoiseFreeObservationsReprojectExactly)
{
  GTEST_SKIP() << "TODO: every generated observation reprojects with zero error";
}

TEST(M0_Synthetic, NoiseAndOutliersAreInjected)
{
  GTEST_SKIP() << "TODO: noise sigma and outlier ratio of the generated data match the requested values";
}
