// Checks that the dependencies compile, link and run. Not part of the SfM code itself.

#include <Eigen/Core>
#include <ceres/ceres.h>
#include <gtest/gtest.h>
#include <opencv2/core.hpp>

namespace
{
struct Quadratic
{
  template <typename T>
  bool operator()(const T *x, T *residual) const
  {
    residual[0] = T(10.0) - x[0];
    return true;
  }
};
} // namespace

TEST(Toolchain, Eigen)
{
  const Eigen::Matrix3d I = Eigen::Matrix3d::Identity();
  EXPECT_DOUBLE_EQ((I * Eigen::Vector3d(1, 2, 3)).sum(), 6.0);
}

TEST(Toolchain, OpenCV)
{
  const cv::Mat m = cv::Mat::eye(3, 3, CV_64F);
  EXPECT_DOUBLE_EQ(cv::sum(m)[0], 3.0);
}

TEST(Toolchain, CeresSolvesHelloWorld)
{
  double x = 0.5;
  ceres::Problem problem;
  problem.AddResidualBlock(new ceres::AutoDiffCostFunction<Quadratic, 1, 1>(new Quadratic), nullptr, &x);
  ceres::Solver::Options options;
  ceres::Solver::Summary summary;
  ceres::Solve(options, &problem, &summary);
  EXPECT_TRUE(summary.IsSolutionUsable());
  EXPECT_NEAR(x, 10.0, 1e-6);
}
