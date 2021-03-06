#ifndef UZH_MATLAB_PORT_CROSS_H_
#define UZH_MATLAB_PORT_CROSS_H_

#include "armadillo"
#include "glog/logging.h"
#include "transform/hat.h"

namespace uzh {

//@brief Imitate matlab's cross. Return the cross product of pairs of vectors
// stored in matrix A and B respectively.
//@param A [3 x n] matrix.
//@param B [3 x n] matrix.
//@return C -- [3 x n] matrix where each column contains the cross product of
// pairs of corresponding columns of A and B.
template <typename T>
arma::Mat<T> /* C */
inline cross(const arma::Mat<T>& A, const arma::Mat<T>& B) {
  if (A.n_rows != 3 || B.n_cols != 3)
    LOG(ERROR) << "cross only supports multiplication between two 3D vectors.";
  if (A.n_cols != B.n_cols)
    LOG(ERROR) << "Number of vectors in A and B must be consistent.";

  const int num_pairs = A.n_cols;
  // Construct matrix C to be populated.
  arma::Mat<T> C(3, num_pairs, arma::fill::zeros);

  // Compute pair-wise cross products.
  for (int i = 0; i < num_pairs; ++i) {
    C.col(i) = uzh::hat<T>(A.col(i)) * B.col(i);
  }

  return C;
}

}  // namespace uzh

#endif  // UZH_MATLAB_PORT_CROSS_H_