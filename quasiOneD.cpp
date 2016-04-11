#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <math.h>
#include <vector>
#include "flux.h"
#include "timestep.h"
#include "globals.h"

double isenP(double pt, double M);

double isenT(double Tt, double M);

std::vector <double> calcVolume(
    std::vector <double> S,
    std::vector <double> dx);

double TotalPressureLoss(std::vector <double> W);

void ioTargetPressure(int io, std::vector <double> &p);

double inverseFitness(
    std::vector <double> pcurrent,
    std::vector <double> ptarget,
    std::vector <double> dx);

void inletBC(
    std::vector <double> &W,
    std::vector <double> &Resi,
    double dt0, double dx0);
void outletBC(
    std::vector <double> &W,
    std::vector <double> &Resi,
    double dt0, double dx0);

double quasiOneD(
    std::vector <double> x,
    std::vector <double> dx,
    std::vector <double> S,
    std::vector <double> &W)
{
    std::vector <double> rho(nx), u(nx), e(nx);
    std::vector <double> T(nx), p(nx), c(nx), Mach(nx);
    std::vector <double> Resi(3 * nx, 0);
//  std::vector <std::vector <double> > W(3, std::vector <double> (nx, 0)),

    std::vector <double> dt(nx), V(nx);

    std::vector <int> itV(maxIt/printIt);
    std::vector <double> normV(maxIt/printIt);

    int iterlength;
    double normR = 1.0;
    int iterations = 0;


    V = calcVolume(S, dx);


    // Inlet flow properties
    Mach[0]=Min;
    T[0] = isenT(Ttin, Mach[0]);
    p[0] = isenP(ptin, Mach[0]);
    rho[0] = p[0] / (R * T[0]);
    c[0] = sqrt(gam * p[0] / rho[0]);
    u[0] = Mach[0] * c[0];
    e[0] = rho[0] * (Cv * T[0] + 0.5 * pow(u[0], 2));

    // Flow Properties Initialization
    for(int i = 1; i < nx; i++)
    {
        Mach[i] = Min;
        p[i] = pexit;
        T[i] = T[0];
        rho[i] = p[i] / (R * T[i]);
        c[i] = sqrt(gam * p[i] / rho[i]);
        u[i] = c[i] * Mach[i];
        e[i] = rho[i] * (Cv * T[i] + 0.5 * pow(u[i], 2));
    }

    // State Vectors Initialization
    for(int i = 0; i < nx; i++)
    {
        W[i * 3 + 0] = rho[i];
        W[i * 3 + 1] = rho[i] * u[i];
        W[i * 3 + 2] = e[i];
    }

    while(normR > conv && iterations < maxIt)
    {
        iterations++;

        if(iterations%printIt == 0)
        {
            if(printConv == 1)
            {
                std::cout<<"Iteration "<<iterations
                         <<"   NormR "<<std::setprecision(15)<<normR<<std::endl;
            }
            itV[iterations / printIt - 1] = iterations;
            normV[iterations / printIt - 1] = normR;
        }

        // Calculate Time Step
        for(int i = 0; i < nx; i++)
            dt[i] = (CFL * dx[i]) / fabs(u[i] + c[i]);

        // Step in Time
        stepInTime(S, dx, dt, Resi, W);

        // Update Inlet BC W[0]
        inletBC(W, Resi, dt[0], dx[0]);

        // Update Oulet BC W[nx - 1]
        outletBC(W, Resi, dt[nx - 1], dx[nx - 1]);

        // Update flow properties
        for(int i = 0; i < nx ; i++)
        {
            rho[i] = W[i * 3 + 0];     // rho
            u[i] = W[i * 3 + 1] / rho[i];  // U
            e[i] = W[i * 3 + 2];       // Energy
            p[i] = (gam - 1) * (e[i] - rho[i] * pow(u[i], 2) / 2);  // Pressure
            T[i] = p[i] / (rho[i] * R); // Temperature
            c[i] = sqrt((gam * p[i]) / rho[i]);// Speed of sound
            Mach[i] = u[i] / c[i];      // Mach number
        }

        // Calculating the norm of the density residual
        normR = 0;
        for(int i = 0; i < nx; i++)
            normR = normR + pow(Resi[i * 3 + 0], 2);
        normR = sqrt(normR);
    }

    if(printW == 1)
    {
        for(int k = 0; k < 3; k++)
        {
            std::cout<<"W"<<k + 1<<std::endl;
            for(int i = 0; i < nx; i++)
                std::cout<<W[i * 3 + k]<<std::endl;
        }
    }
    std::cout<<"Flow iterations = "<<iterations<<"   Density Residual = "<<normR<<std::endl;


    FILE  * Results;
    Results = fopen("Results.dat", "w");
    fprintf(Results, "%d\n", nx);
    for(int i = 0; i < nx; i++)
        fprintf(Results, "%.15f\n", x[i]);
    for(int i = 0; i < nx; i++)
        fprintf(Results, "%.15f\n", p[i] / ptin);
    for(int i = 0; i < nx; i++)
        fprintf(Results, "%.15f\n", rho[i]);
    for(int i = 0; i < nx; i++)
        fprintf(Results, "%.15f\n", Mach[i]);
    for(int i = 0; i < nx; i++)
        fprintf(Results, "%.15f\n", x[i] - dx[i] / 2);
    fprintf(Results, "%f\n", x.back() + dx.back() / 2);
    for(int i = 0; i < nx + 1; i++)
        fprintf(Results, "%.15f\n", S[i]);

    iterlength = itV.size();
    for(int i = 0; i < iterlength; i++)
        fprintf(Results, "%.15d\n", itV[i]);
    for(int i = 0; i < iterlength; i++)
        fprintf(Results, "%.15f\n", normV[i]);



    fclose(Results);

    // Create Target Pressure
    if(createTarget == 1) ioTargetPressure(1, p);

    // Compute Fitness
    if(fitnessFun == 0)
    {
        double tpl = TotalPressureLoss(W);
        std::cout<<"Total Pressure Loss: "<<tpl<<std::endl;
        return tpl;
    }
    else if(fitnessFun == 1)
    {
        std::vector <double> ptarget(nx, 0);
        ioTargetPressure(-1, ptarget);
        return inverseFitness(p, ptarget, dx);
    }



    return  - 9999.99;

}

double isenP(double pt, double M)
{
    return pt * pow((1 + (gam - 1) / 2 * pow(M, 2)), ( - gam / (gam - 1)));
}

double isenT(double Tt, double M)
{
    return Tt * pow((1 + (gam - 1) / 2 * pow(M, 2)), - 1);
}

double TotalPressureLoss(std::vector <double> W)
{
    double rhoout = W[(nx - 1) * 3 + 0];
    double uout = W[(nx - 1) * 3 + 1] / rhoout;
    double pout = (gam - 1) * (W[(nx - 1) * 3 + 2] - rhoout * pow(uout, 2) / 2);
    //double Tout = pout/(rhoout * R);

    double ptout_normalized;

    double ToverTt = 1 - pow(uout, 2) / a2 * (gam - 1) / (gam + 1);

    double poverpt = pow(ToverTt, (gam / (gam - 1)));

    ptout_normalized = 1 - (pout / poverpt) / ptin;

    return ptout_normalized;
}

// Define Volume
std::vector <double> calcVolume(
    std::vector <double> S,
    std::vector <double> dx)
{
    std::vector <double> V;
    int ndx = dx.size();
    for(int i = 0; i < ndx; i++)
        V.push_back((S[i] + S[i + 1]) / 2 * dx[i]);

    return V;
}

// Input/Output Target Pressure Distribution
void ioTargetPressure(int io, std::vector <double> &p)
{

    FILE  * TargetP;
    int err;
    // Output
    if(io > 0)
    {
        TargetP = fopen("targetP.dat", "w");
        fprintf(TargetP, "%d\n", nx);
//      for(int i = 0; i < nx; i++)
//          fprintf(TargetP, "%.15f\n", x[i]);
        for(int i = 0; i < nx; i++)
            fprintf(TargetP, "%.15f\n", p[i] / ptin);
    }
    // Input
    else
    {
        int nxT;

        TargetP = fopen("targetP.dat", "r");
        rewind(TargetP);
        err = fscanf(TargetP, "%d", &nxT);
        if(nxT!=nx) std::cout<< "nx and nxT are different for targetP";
        for(int iT = 0; iT < nxT; iT++)
        {
            err = fscanf(TargetP, "%lf", &p[iT]);
        }
        if(err != 1) std::cout<< "Err";
    }

    fclose(TargetP);
}

// Return Inverse Design Fitness

double inverseFitness(
    std::vector <double> pcurrent,
    std::vector <double> ptarget,
    std::vector <double> dx)
{
    double fit = 0;
    for(int i = 0; i < nx; i++)
    {
        fit += pow(pcurrent[i] / ptin - ptarget[i], 2) * dx[i];
    }
//  std::cout<<"InverseFitness =  "<<fit / 2<<std::endl;
    return fit / 2;
}


void inletBC(
    std::vector <double> &W,
    std::vector <double> &Resi,
    double dt0, double dx0)
{
    double dpdu, dpdx, dudx, dtdx, du, eigenvalue, T0;
    double rho[2], u[2], e[2], p[2], c[2];
    for(int i = 0; i < 2; i++)
    {
        rho[i] = W[i * 3 + 0];
        u[i] = W[i * 3 + 1] / rho[i];
        e[i] = W[i * 3 + 2];
        p[i] = (gam - 1) * ( e[i] - rho[i] * u[i] * u[i] / 2 );
        c[i] = sqrt( gam * p[i] / rho[i] );
    }
    if(u[0] < c[0])
    {
        dpdu = ptin * (gam / (gam - 1.0))
             * pow(1.0 - ((gam - 1.0) / (gam + 1.0)) * u[0] * u[0] / a2,
                   1.0 / (gam - 1.0))
             * ( - 2.0 * ((gam - 1.0) / (gam + 1.0)) * u[0] / a2);

        dtdx = dt0 / dx0;
        eigenvalue = ((u[1] + u[0] - c[1] - c[0]) / 2.0) * dtdx;

        dpdx = p[1] - p[0];
        dudx = u[1] - u[0];
        du = -eigenvalue * (dpdx - rho[0] * c[0] * dudx)
                / (dpdu - rho[0] * c[0]);

//      Resi[0 * 3 + 1] = -((u[0] + du) - u[0]) / dtdx;
        u[0] = u[0] + du;

        T0 = Ttin * (1.0 - ((gam - 1.0) / (gam + 1.0)) * u[0] * u[0] / a2);
//      Resi[0 * 3 + 2] = -(ptin * pow(T0 / Ttin, gam / (gam - 1.0)) - p[0]) / dtdx;
        p[0] = ptin * pow(T0 / Ttin, gam / (gam - 1.0));
        rho[0] = p[0] / (R * T0);
        e[0] = rho[0] * (Cv * T0 + 0.5 * u[0] * u[0]);

        Resi[0 * 3 + 0] = -(rho[0] - W[0 * 3 + 0]) / dtdx;
        Resi[0 * 3 + 1] = -(rho[0] * u[0] - W[0 * 3 + 1]) / dtdx;
        Resi[0 * 3 + 2] = -(e[0] - W[0 * 3 + 2]) / dtdx;

        W[0 * 3 + 0] = rho[0];
        W[0 * 3 + 1] = rho[0] * u[0];
        W[0 * 3 + 2] = e[0];
    }
    else
    {
        Resi[0 * 3 + 0] = 0;
        Resi[0 * 3 + 1] = 0;
        Resi[0 * 3 + 2] = 0;
    }
}

void outletBC(
    std::vector <double> &W,
    std::vector <double> &Resi,
    double dt0, double dx0)
{
    double avgc, avgu, dtdx, MachOut;
    double eigenvalues[3], Ri[3];
    double dpdx, dudx, du, drho, dp, T;
    double rho[2], u[2], e[2], p[2], c[2];

    for(int i = 0; i < 2; i++)
    {
        rho[i] = W[(i + (nx - 2)) * 3 + 0];
        u[i] = W[(i + (nx - 2)) * 3 + 1] / rho[i];
        e[i] = W[(i + (nx - 2)) * 3 + 2];
        p[i] = (gam - 1) * ( e[i] - rho[i] * u[i] * u[i] / 2 );
        c[i] = sqrt( gam * p[i] / rho[i] );
    }

    // Exit boundary condition
    avgu = (u[1] + u[0]) / 2;
    avgc = (c[1] + c[0]) / 2;
    dtdx = dt0 / dx0;
    eigenvalues[0] = avgu * dtdx;
    eigenvalues[1] = (avgu + avgc) * dtdx;
    eigenvalues[2] = (avgu - avgc) * dtdx;

    dpdx = p[1] - p[0];
    dudx = u[1] - u[0];

    Ri[0] = -eigenvalues[0] * ( rho[1] - rho[0] - dpdx / pow(c[1], 2) );
    Ri[1] = -eigenvalues[1] * ( dpdx + rho[1] * c[1] * dudx );
    Ri[2] = -eigenvalues[2] * ( dpdx - rho[1] * c[1] * dudx );

    MachOut = avgu / avgc;
    if(MachOut > 1)
    {
        dp = 0.5 * (Ri[1] + Ri[2]);
    }
    else
    {
        dp = 0;
    }

    drho = Ri[0] + dp / (pow(c[1], 2));
    du = (Ri[1] - dp) / (rho[1] * c[1]);

    u[1] = u[1] + du;
    rho[1] = rho[1] + drho;
    p[1] = p[1] + dp;
    T = p[1] / (rho[1] * R);
    e[1] = rho[1] * (Cv * T + 0.5 * pow(u[1], 2));

    Resi[(nx - 1) * 3 + 0] = (W[(nx - 1) * 3 + 0] - rho[1]) / dtdx;
    Resi[(nx - 1) * 3 + 1] = (W[(nx - 1) * 3 + 1] - rho[1] * u[1]) / dtdx;
    Resi[(nx - 1) * 3 + 2] = (W[(nx - 1) * 3 + 2] - e[1]) / dtdx;

    W[(nx - 1) * 3 + 0] = rho[1];
    W[(nx - 1) * 3 + 1] = rho[1] * u[1];
    W[(nx - 1) * 3 + 2] = e[1];
}
