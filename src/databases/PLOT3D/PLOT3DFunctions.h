// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     PLOT3DFunctions.h                                     //
// ************************************************************************* //


#ifndef PLOT3DFunctions_h
#define PLOT3DFunctions_h

// ****************************************************************************
//  namespace: PLOT3DFunctions
//
//  Purpose:
// Computes functions based on values from the PLOT3D solution file
// 
// ****************************************************************************

template <class DataType>
class PLOT3DFunctions
{
public:
    void ComputeTemperature(
      int numPts,
      DataType *temperature,
      DataType *Density,
      DataType *Momentum,
      DataType *Energy,
      double R, double Gamma);

    void ComputePressure(
      int numPts,
      DataType *pressure,
      DataType *Density,
      DataType *Momentum,
      DataType *Energy,
      double Gamma);

    void ComputeEnthalpy(
      int numPts,
      DataType *enthalpy,
      DataType *Density,
      DataType *Momentum,
      DataType *Energy,
      double Gamma);

    void ComputeKineticEnergy(
      int numPts,
      DataType *kineticEnergy,
      DataType *Density,
      DataType *Momentum);

    void ComputeEntropy(
      int numPts,
      DataType *entropy,
      DataType *Density,
      DataType *Momentum,
      DataType *Energy,
      double R,
      double Gamma);

    void ComputeSwirl(
      int numPts,
      DataType *swirl,
      DataType *Density,
      DataType *Momentum,
      DataType *Vorticity);

    void ComputeVelocity(
      int numPts,
      DataType *velocity,
      DataType *Density,
      DataType *Momentum);

    void ComputeVorticity(
      int numPts,
      DataType *vorticity,
      DataType *Points,
      DataType *Velocity,
      int *dims);

    void ComputePressureGradient(
      int numPts,
      DataType *gradient,
      DataType *Points,
      DataType *Pressure,
      int *dims);

    void ComputePressureCoefficient(
      int numPts,
      DataType *pressure_coeff,
      DataType *Density,
      DataType *Momentum,
      DataType *Energy,
      DataType *Gamma,
      DataType gi,
      DataType fsm);

    void ComputeMachNumber(
      int numPts,
      DataType *machnumber,
      DataType *Density,
      DataType *Momentum,
      DataType *Energy,
      DataType *Gamma);

    void ComputeSoundSpeed(
      int numPts,
      DataType *soundspeed,
      DataType *Density,
      DataType *Momentum,
      DataType *Energy,
      DataType *Gamma);

    void ComputeStrainRate(
      int numPts,
      DataType *strainRate,
      DataType *Points,
      DataType *Velocity,
      int *dims);

};


#endif
