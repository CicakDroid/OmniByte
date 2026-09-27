#include "Eigen.h"

namespace omnibyte {
namespace common {

VectorXd solve_linear(const MatrixXd& A, const VectorXd& b) {
  if (A.rows() != A.cols() || A.rows() != b.size() || A.rows() == 0) {
    return VectorXd();
  }
  Eigen::FullPivLU<MatrixXd> lu(A);
  if (!lu.isInvertible()) {
    return VectorXd();
  }
  return lu.solve(b);
}

}  // namespace common
}  // namespace omnibyte
