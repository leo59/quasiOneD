#include <math.h>
#include "globals.h"
#include <string>
std::string fname()
{
    static std::string fn("./Results/run");
    return fn;
}
const double PI = atan(1.0) * 4.0;

int nx;
double a_geom, b_geom;
double h_geom, t1_geom, t2_geom;

int StepScheme, FluxScheme;
double Scalareps;

double CFL;
double flowConv;
int maxIt;

int printIt, printConv, printW;

double gam, R, Cv;
double Min, Ttin, ptin, pexit;
double a2;

int opt, desParam, fitnessFun;
int nDesVar;
int descentType, gradientType, hessianType, exactHessian;
double gradConv;
int maxDesign;

double h_tar, t1_tar, t2_tar;
