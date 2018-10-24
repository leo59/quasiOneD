#ifndef CONVERT_H
#define CONVERT_H

#include<vector>
#include<Eigen/Core>
#include<adolc/adolc.h>

// Get pressure
template<typename dreal>
inline dreal get_p(const double gamma, const dreal rho, const dreal rho_u, const dreal e) {
	return (gamma - 1.0) * ( e - (pow(rho_u, 2.0) / rho+1e-14) / 2.0 );
};
template double get_p(const double gamma, const double rho, const double rho_u, const double e);
template adouble get_p(const double gamma, const adouble rho, const adouble rho_u, const adouble e);

template<typename dreal>
inline dreal get_c(const double gamma, const dreal rho, const dreal rho_u, const dreal e) {
	const dreal p = get_p(gamma, rho, rho_u, e);
	dreal a =  gamma / rho * p;
	if(a<0.0) {
		//printf("Negative radicand of sqrt. Either p: %4.2f or rho: %4.2f", p, rho)
	}
	return sqrt(a);
	//return sqrt( gamma / rho * get_p(gamma, rho, rho_u, e) );
};
template double get_c(const double gamma, const double rho, const double rho_u, const double e);
template adouble get_c(const double gamma, const adouble rho, const adouble rho_u, const adouble e);

template<typename dreal>
inline dreal isenP(const double gam, const double pt, const dreal M) {
	return pt * pow((1 + (gam - 1) / 2 * pow(M, 2)), ( - gam / (gam - 1)));
};
template double isenP(const double gam, const double pt, const double M);
template adouble isenP(const double gam, const double pt, const adouble M);

template<typename dreal>
inline dreal isenT(const double gam, const double Tt, const dreal M) {
	return Tt * pow((1 + (gam - 1) / 2 * pow(M, 2)), - 1);
};
template double isenT(const double gam, const double Tt, const double M);
template adouble isenT(const double gam, const double Tt, const adouble M);

void get_all_p(
	const double gam,
    std::vector<double> const &W,
    std::vector<double> &p);

void eval_dWpdW(
	const double gam,
	const double rho,
	const double rho_u,
    std::vector<double>* const dWpdW_vec);

Eigen::MatrixXd eval_dWpdW(
	const double gam,
	const double rho,
	const double rho_u);

void dWdWp(
	const double gam,
    std::vector<double> &M,
    std::vector<double> const &W,
    int k);

void WtoF(
	const double gam,
    std::vector<double> const &W,
    std::vector<double> &F);

void WtoQ(
	const double gam,
    const std::vector<double> &area,
    const std::vector<double> &W,
    std::vector<double> &Q);

std::vector <Eigen::Matrix3d> ddWpdWdWp(
    const double gam,
    const std::vector<double> &W,
    const int i);
#endif