#ifndef SOLVER_LINEAR_H
#define SOLVER_LINEAR_H

#include <Eigen/Core>
#include <Eigen/SparseCore>
#include <vector>
#include "structures.h"

using namespace Eigen;
MatrixXd solve_linear(
    const SparseMatrix<double> A,
    const MatrixXd B,
	const int linear_solver_type,
	const double tolerance);
VectorXd solve_linear(
    const SparseMatrix<double> A,
    const VectorXd B,
	const int linear_solver_type,
	const double tolerance);
VectorXd solve_linear(
    const MatrixXd A,
    const VectorXd rhs,
	const int linear_solver_type,
	const double tolerance);
#endif
