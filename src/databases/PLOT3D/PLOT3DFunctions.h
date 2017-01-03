/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
