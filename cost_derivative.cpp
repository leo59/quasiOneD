#include<vector>
#include<Eigen/Core>
#include<Eigen/Sparse>
#include"fitness.h"
#include<iostream>

using namespace Eigen;

// dCost / dW
VectorXd evaldCostdW(
	const Optimization_options &opt_opts,
	const Flow_options &flo_opts,
	const std::vector<double> &W,
	const std::vector<double> &dx)
{
	const int n_elem = W.size()/3;
	const double inlet_total_p = flo_opts.inlet_total_p;
	const double gam = flo_opts.gam;

    VectorXd dCostdW(3 * n_elem);

    for (int i = 0; i < n_elem; i++)
    {
        const double rho = W[i * 3 + 0];
        const double u = W[i * 3 + 1] / rho;
        const double p = (gam - 1) * ( W[i * 3 + 2] - rho * u * u / 2.0 );

        const double dpdw0 = (gam - 1) / 2.0 * u * u;
        const double dpdw1 = - (gam - 1) * u;
        const double dpdw2 = (gam - 1);

        dCostdW[i * 3 + 0] = (p / inlet_total_p - opt_opts.target_pressure[i]) * dpdw0 * dx[i] / inlet_total_p;
        dCostdW[i * 3 + 1] = (p / inlet_total_p - opt_opts.target_pressure[i]) * dpdw1 * dx[i] / inlet_total_p;
        dCostdW[i * 3 + 2] = (p / inlet_total_p - opt_opts.target_pressure[i]) * dpdw2 * dx[i] / inlet_total_p;
    }
    return dCostdW;
}

// ddCost / dWdW
SparseMatrix<double> evaldCostdWdW(
	const Optimization_options &opt_opts,
	const Flow_options &flo_opts,
	std::vector<double> W,
	std::vector<double> dx)
{
	const int n_elem = W.size()/3;
	const double inlet_total_p = flo_opts.inlet_total_p;
	const double gam = flo_opts.gam;
    SparseMatrix<double> ddCostdWdW(3 * n_elem, 3 * n_elem);
    Matrix3d ddCostdWdW_temp = Matrix3d::Zero();
    Matrix3d ddpdWdW = Matrix3d::Zero();
    Vector3d dpdW = Vector3d::Zero();

    for (int Wi = 0; Wi < n_elem; Wi++)
    {
        const double rho = W[Wi * 3 + 0];
        const double u = W[Wi * 3 + 1] / rho;
        const double p = (gam - 1) * ( W[Wi * 3 + 2] - rho * u * u / 2.0 );

        dpdW(0) = (gam - 1) / 2.0 * u * u;
        dpdW(1) = - (gam - 1) * u;
        dpdW(2) = (gam - 1);
        
        ddpdWdW(0, 0) = -u * u * (gam - 1.0) / rho;
        ddpdWdW(1, 1) = (1.0 - gam) / rho;
        ddpdWdW(1, 0) = u * (gam - 1.0) / rho;
        ddpdWdW(0, 1) = ddpdWdW(1, 0);

        const double dxptin2 = dx[Wi] / pow(inlet_total_p, 2);
        ddCostdWdW_temp = dxptin2 * dpdW * dpdW.transpose()
                        + (dxptin2 * p - opt_opts.target_pressure[Wi] * dx[Wi] / inlet_total_p) * ddpdWdW;

        for (int ki = 0; ki < 3; ki++)
        {
            for (int kj = 0; kj < 3; kj++)
            {
                ddCostdWdW.insert(Wi*3+ki, Wi*3+kj) = ddCostdWdW_temp(ki, kj);
            }
        }
    }
    return ddCostdWdW;
}

VectorXd evaldCostdS(const int n_elem) {
    VectorXd dCostdS(n_elem + 1);
    dCostdS.setZero();
    return dCostdS;
}

MatrixXd evalddCostdSdS(const int n_elem) {
    MatrixXd ddCostdSdS(n_elem + 1, n_elem + 1);
    ddCostdSdS.setZero();
    return ddCostdSdS;
}

MatrixXd evalddCostdWdS(const int n_elem) {
    MatrixXd ddCostdWdS(3 * n_elem, n_elem + 1);
    ddCostdWdS.setZero();
    return ddCostdWdS;
}
