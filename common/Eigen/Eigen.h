// Eigen adapter — minimal shared wrapper (MatrixXd/VectorXd + one solver).
// Keep tiny; Hydra2D has no consumer today. Do not grow this without a caller.
#ifndef OMNIBYTE_COMMON_EIGEN_H
#define OMNIBYTE_COMMON_EIGEN_H

#include <eigen5/Eigen/Dense>

namespace omnibyte {
namespace common {

using MatrixXd = Eigen::MatrixXd;
using VectorXd = Eigen::VectorXd;
using Matrix2d = Eigen::Matrix2d;
using Vector2d = Eigen::Vector2d;
using Matrix3d = Eigen::Matrix3d;
using Vector3d = Eigen::Vector3d;
using Index = Eigen::Index;

// Solve Ax = b. Returns (n) solution, or empty vector if A is singular.
VectorXd solve_linear(const MatrixXd& A, const VectorXd& b);

}  // namespace common
}  // namespace omnibyte

#endif  // OMNIBYTE_COMMON_EIGEN_H
