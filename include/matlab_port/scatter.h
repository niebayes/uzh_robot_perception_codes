#ifndef UZH_MATLAB_PORT_SCATTER_H_
#define UZH_MATLAB_PORT_SCATTER_H_

#include "Eigen/Core"
#include "armadillo"
#include "glog/logging.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"

namespace uzh {

//@brief Imitate matlab's scatter.
// TODO(bayes) Templatize this function to make the parameter parsing more
// flexible.
static void scatter(cv::Mat& image, const Eigen::Ref<const Eigen::VectorXi>& x,
                    const Eigen::Ref<const Eigen::VectorXi>& y,
                    const int radius, const cv::Scalar& color,
                    const int thickness = 1) {
  if (x.size() <= 0 || y.size() <= 0) {
    LOG(ERROR) << "Invalid input vectors";
    return;
  } else if (x.size() != y.size()) {
    LOG(ERROR)
        << "YOU_MIXED_DIFFERENT_SIZED_VECTORS";  // Mimic eigen's behavior
    return;
  }

  const int kNumPoints = x.size();
  for (int i = 0; i < kNumPoints; ++i) {
    cv::circle(image, {x(i), y(i)}, radius, color, thickness);
  }
}

//@brief Overloaded for armadillo.
static void scatter(cv::Mat& image, const arma::uvec& x, const arma::uvec& y,
                    const int radius, const cv::Scalar& color,
                    const int thickness = 1) {
  if (x.size() <= 0 || y.size() <= 0) {
    LOG(ERROR) << "Invalid input vectors";
    return;
  } else if (x.size() != y.size()) {
    LOG(ERROR) << "x and y must contain the same number of entries.";
    return;
  }

  const int kNumPoints = x.size();
  for (int i = 0; i < kNumPoints; ++i) {
    cv::circle(image, {static_cast<int>(x(i)), static_cast<int>(y(i))}, radius,
               color, thickness);
  }
}

}  // namespace uzh

#endif  // UZH_MATLAB_PORT_SCATTER_H_