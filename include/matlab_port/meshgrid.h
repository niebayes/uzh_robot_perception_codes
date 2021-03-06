#ifndef UZH_MATLAB_PORT_MESHGRID_H_
#define UZH_MATLAB_PORT_MESHGRID_H_

#include <tuple>
#include <vector>

#include "Eigen/Core"
#include "armadillo"
#include "opencv2/core.hpp"

namespace uzh {

//@brief Imitate matlab's meshgrid.
// TODO Generalize this function to arbitrarily accomodating [low, hight] range
// values. E.g. use OpenCV's cv::Range
template <typename Derived>
void meshgrid(const int width, const int height, Eigen::MatrixBase<Derived>* X,
              Eigen::MatrixBase<Derived>* Y) {
  const Eigen::VectorXi x = Eigen::VectorXi::LinSpaced(width, 0, width - 1),
                        y = Eigen::VectorXi::LinSpaced(height, 0, height - 1);
  *X = x.transpose().replicate(height, 1);
  *Y = y.replicate(1, width);
}

//@brief Overloaded for arma::mat
template <typename T>
std::tuple<arma::Mat<T> /*X*/, arma::Mat<T> /*Y*/> meshgrid(const int width,
                                                            const int height) {
  const arma::Col<T> x = arma::linspace<arma::Col<T>>(0, width - 1, width),
                     y = arma::linspace<arma::Col<T>>(0, height - 1, height);
  const arma::Mat<T> X = arma::repmat(x.t(), height, 1),
                     Y = arma::repmat(y, 1, width);
  return {X, Y};
}

//@brief Imitate matlab's meshgrid operating on 3D grid though.
// You could also use eigen's Tensor module which is not supported yet though.
//@ref
// http://eigen.tuxfamily.org/dox-devel/unsupported/group__CXX11__Tensor__Module.html
//@warning If using fixed-size eigen objects, care has to be taken on the
// alignment issues.
//@ref https://eigen.tuxfamily.org/dox/group__TopicStlContainers.html
//? Why "template argument deduction failed"?
// template <typename Derived>
void meshgrid(const cv::Range& x_range, const cv::Range& y_range,
              const cv::Range& z_range, std::vector<Eigen::MatrixXi>* X,
              std::vector<Eigen::MatrixXi>* Y,
              std::vector<Eigen::MatrixXi>* Z) {
  //  std::vector<typename Eigen::MatrixBase<Derived>>* X,
  //  std::vector<typename Eigen::MatrixBase<Derived>>* Y,
  //  std::vector<typename Eigen::MatrixBase<Derived>>* Z) {
  const int width = x_range.size() + 1, height = y_range.size() + 1,
            depth = z_range.size() + 1;
  const Eigen::VectorXi x = Eigen::VectorXi::LinSpaced(width, x_range.start,
                                                       x_range.end),
                        y = Eigen::VectorXi::LinSpaced(height, y_range.start,
                                                       y_range.end),
                        z = Eigen::VectorXi::LinSpaced(depth, z_range.start,
                                                       z_range.end);
  // const Eigen::MatrixBase<Derived>
  const Eigen::MatrixXi X_any_depth = x.transpose().replicate(height, 1),
                        Y_any_depth = y.replicate(1, width);
  for (int d = 0; d < depth; ++d) {
    X->push_back(X_any_depth);
    Y->push_back(Y_any_depth);
    Eigen::MatrixXi Z_d_depth(height, width);
    Z_d_depth.fill(z(d));
    Z->push_back(Z_d_depth);
  }
}

}  // namespace uzh

#endif  // UZH_MATLAB_PORT_MESHGRID_H_