#ifndef gradient_h
#define gradient_h
#include <Eigen/Core>
#include <Eigen/SparseCore>
#include <vector>
#include "structures.h"

using namespace Eigen;
VectorXd getGradient(
	const int gradient_type,
	const int cost_function,
    const std::vector<double> &x,
    const std::vector<double> &dx,
    const std::vector<double> &area,
	const struct Flow_options &flow_options,
	const struct Flow_data &flow_data,
	const struct Optimization_options &opt_opts,
	const struct Design &design);

MatrixXd evaldWdDes(
    const std::vector<double> &x,
    const std::vector<double> &dx,
    const std::vector<double> &area,
	const struct Flow_options &flow_options,
	const struct Flow_data &flow_data,
	const struct Design &design);

MatrixXd solveSparseAXB(
    const SparseMatrix<double> A,
    const MatrixXd b,
    const int eig_solv);
#endif
