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
//                     PLOT3DFunctions.C                                     //
// ************************************************************************* //


//
// The calculations used here are adapted from vtkMultiBlockPLOT3DReader,
// VTK Version 6.1, by Kitware, http://www.vtk.org
//
// They were templatized to work on native data arrays.
// The actual calculations have not been changed, only how data is
// passed to and from.
//

#include "PLOT3DFunctions.h"
#include <cmath>

template <class DataType>
void PLOT3DFunctions<DataType>::ComputeTemperature(
    int numPts,
    DataType *temperature,
    DataType *Density,
    DataType *Momentum,
    DataType *Energy,
    double R, double Gamma)
{
  double m[3], e, rr, u, v, w, v2, p, d, rrgas;

  //  Compute the temperature
  //
  rrgas = 1.0 / R;
  for (int i=0; i < numPts; ++i)
    {
    d = Density[i];
    d = (d != 0.0 ? d : 1.0);
    m[0] = Momentum[3*i+0];
    m[1] = Momentum[3*i+1];
    m[2] = Momentum[3*i+2];
    e = Energy[i];
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    p = (Gamma-1.) * (e - 0.5 * d * v2);
    temperature[i] = p*rr*rrgas;
  }
}

template <class DataType>
void PLOT3DFunctions<DataType>::ComputePressure(
    int numPts,
    DataType *pressure,
    DataType *Density,
    DataType *Momentum,
    DataType *Energy,
    double Gamma)
{
  double m[3], e, u, v, w, v2, d, rr;

  //  Compute the pressure
  //
  for (int i=0; i < numPts; ++i)
    {
    d = Density[i];
    d = (d != 0.0 ? d : 1.0);
    m[0] = Momentum[3*i+0];
    m[1] = Momentum[3*i+1];
    m[2] = Momentum[3*i+2];
    e = Energy[i];
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    pressure[i] = (Gamma-1.) * (e - 0.5 * d * v2);
  }
}

template<class DataType>
void PLOT3DFunctions<DataType>::ComputeEnthalpy(
    int numPts,
    DataType *enthalpy,
    DataType *Density,
    DataType *Momentum,
    DataType *Energy,
    double Gamma)
{
  double m[3], e, u, v, w, v2, d, rr;

  //  Compute the enthalpy
  //
  for (int i=0; i < numPts; ++i)
    {
    d = Density[i];
    d = (d != 0.0 ? d : 1.0);
    m[0] = Momentum[3*i+0];
    m[1] = Momentum[3*i+1];
    m[2] = Momentum[3*i+2];
    e = Energy[i];
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    enthalpy[i] = Gamma*(e*rr - 0.5*v2);
  }
}

template <class DataType>
void PLOT3DFunctions<DataType>::ComputeKineticEnergy(
    int numPts,
    DataType *kineticEnergy,
    DataType *Density,
    DataType *Momentum)
{
  double m[3], u, v, w, v2, d, rr;

  //  Compute the kinetic this->Energy
  //
  for (int i=0; i < numPts; ++i)
    {
    d = Density[i];
    d = (d != 0.0 ? d : 1.0);
    m[0] = Momentum[3*i+0];
    m[1] = Momentum[3*i+1];
    m[2] = Momentum[3*i+2];
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    kineticEnergy[i] = 0.5*v2;
  }
}

template <class DataType>
void PLOT3DFunctions<DataType>::ComputeEntropy(
    int numPts,
    DataType *entropy,
    DataType *Density,
    DataType *Momentum,
    DataType *Energy,
    double R,
    double Gamma)
{
  double m[3], u, v, w, v2, d, rr, p, e;

  //  Compute the entropy
  //
  for (int i=0; i < numPts; ++i)
    {
    d = Density[i];
    d = (d != 0.0 ? d : 1.0);
    m[0] = Momentum[3*i+0];
    m[1] = Momentum[3*i+1];
    m[2] = Momentum[3*i+2];
    e = Energy[i];
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    p = (Gamma-1.)*(e - 0.5*d*v2);
    //s = VTK_CV * log((p/VTK_PINF)/pow((double)d/VTK_RHOINF,(double)this->Gamma));
    entropy[i] = (R/(Gamma-1.0)) * log((p/(1.0/Gamma))/pow(d/1.0,Gamma));
  }
}

template <class DataType>
void PLOT3DFunctions<DataType>::ComputeSwirl(
    int numPts,
    DataType *swirl,
    DataType *Density,
    DataType *Momentum,
    DataType *Vorticity)
{
  double d, rr, m[3], u, v, w, v2, vort[3], s;

  //
  //  Compute the swirl
  //
  for (int i=0; i < numPts; i++)
    {
    d = Density[i];
    d = (d != 0.0 ? d : 1.0);
    m[0] = Momentum[3*i+0];
    m[1] = Momentum[3*i+1];
    m[2] = Momentum[3*i+2];
    vort[0] = Vorticity[3*i+0];
    vort[1] = Vorticity[3*i+1];
    vort[2] = Vorticity[3*i+2];
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    if ( v2 != 0.0 )
      {
      s = (vort[0]*m[0] + vort[1]*m[1] + vort[2]*m[2]) / v2;
      }
    else
      {
      s = 0.0;
      }

    swirl[i] = s;
  }
}

// Vector functions

template <class DataType>
void PLOT3DFunctions<DataType>::ComputeVelocity(
    int numPts,
    DataType *velocity,
    DataType *Density,
    DataType *Momentum)
{
  double m[3], d, rr;

  //  Compute the velocity
  //
  for (int i=0; i < numPts; i++)
    {
    d = Density[i];
    d = (d != 0.0 ? d : 1.0);
    m[0] = Momentum[3*i+0];
    m[1] = Momentum[3*i+1];
    m[2] = Momentum[3*i+2];
    rr = 1.0 / d;
    velocity[3*i+0] = m[0] * rr;
    velocity[3*i+1] = m[1] * rr;
    velocity[3*i+2] = m[2] * rr;
  }
}

template <class DataType>
void PLOT3DFunctions<DataType>::ComputeVorticity(
    int numPts,
    DataType *vorticity,
    DataType *Points,
    DataType *Velocity,
    int *dims)
{
  int ijsize;
  int i, j, k, idx, idx2, ii, z;
  double vort[3], xp[3], xm[3], vp[3], vm[3], factor;
  double xxi, yxi, zxi, uxi, vxi, wxi;
  double xeta, yeta, zeta, ueta, veta, weta;
  double xzeta, yzeta, zzeta, uzeta, vzeta, wzeta;
  double aj, xix, xiy, xiz, etax, etay, etaz, zetax, zetay, zetaz;

  ijsize = dims[0]*dims[1];

  for (k=0; k<dims[2]; k++)
    {
    for (j=0; j<dims[1]; j++)
      {
      for (i=0; i<dims[0]; i++)
        {
        //  Xi derivatives.
        if ( dims[0] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            vp[ii] = vm[ii] = xp[ii] = xm[ii] = 0.0;
            }
          xp[0] = 1.0;
          }
        else if ( i == 0 )
          {
          factor = 1.0;
          idx = (i+1) + j*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }
        else if ( i == (dims[0]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i-1 + j*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }
        else
          {
          factor = 0.5;
          idx = (i+1) + j*dims[0] + k*ijsize;
          idx2 = (i-1) + j*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }

        xxi = factor * (xp[0] - xm[0]);
        yxi = factor * (xp[1] - xm[1]);
        zxi = factor * (xp[2] - xm[2]);
        uxi = factor * (vp[0] - vm[0]);
        vxi = factor * (vp[1] - vm[1]);
        wxi = factor * (vp[2] - vm[2]);

        //  Eta derivatives.
        if ( dims[1] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            vp[ii] = vm[ii] = xp[ii] = xm[ii] = 0.0;
            }
          xp[1] = 1.0;
          }
        else if ( j == 0 )
          {
          factor = 1.0;
          idx = i + (j+1)*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }
        else if ( j == (dims[1]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i + (j-1)*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }
        else
          {
          factor = 0.5;
          idx = i + (j+1)*dims[0] + k*ijsize;
          idx2 = i + (j-1)*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }


        xeta = factor * (xp[0] - xm[0]);
        yeta = factor * (xp[1] - xm[1]);
        zeta = factor * (xp[2] - xm[2]);
        ueta = factor * (vp[0] - vm[0]);
        veta = factor * (vp[1] - vm[1]);
        weta = factor * (vp[2] - vm[2]);

        //  Zeta derivatives.
        if ( dims[2] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            vp[ii] = vm[ii] = xp[ii] = xm[ii] = 0.0;
            }
          xp[2] = 1.0;
          }
        else if ( k == 0 )
          {
          factor = 1.0;
          idx = i + j*dims[0] + (k+1)*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }
        else if ( k == (dims[2]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + (k-1)*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }
        else
          {
          factor = 0.5;
          idx = i + j*dims[0] + (k+1)*ijsize;
          idx2 = i + j*dims[0] + (k-1)*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }

        xzeta = factor * (xp[0] - xm[0]);
        yzeta = factor * (xp[1] - xm[1]);
        zzeta = factor * (xp[2] - xm[2]);
        uzeta = factor * (vp[0] - vm[0]);
        vzeta = factor * (vp[1] - vm[1]);
        wzeta = factor * (vp[2] - vm[2]);

        // Now calculate the Jacobian.  Grids occasionally have
        // singularities, or points where the Jacobian is infinite (the
        // inverse is zero).  For these cases, we'll set the Jacobian to
        // zero, which will result in a zero vorticity.
        //
        aj =  xxi*yeta*zzeta+yxi*zeta*xzeta+zxi*xeta*yzeta
              -zxi*yeta*xzeta-yxi*xeta*zzeta-xxi*zeta*yzeta;
        if (aj != 0.0)
          {
          aj = 1. / aj;
          }

        //  Xi metrics.
        xix  =  aj*(yeta*zzeta-zeta*yzeta);
        xiy  = -aj*(xeta*zzeta-zeta*xzeta);
        xiz  =  aj*(xeta*yzeta-yeta*xzeta);

        //  Eta metrics.
        etax = -aj*(yxi*zzeta-zxi*yzeta);
        etay =  aj*(xxi*zzeta-zxi*xzeta);
        etaz = -aj*(xxi*yzeta-yxi*xzeta);

        //  Zeta metrics.
        zetax=  aj*(yxi*zeta-zxi*yeta);
        zetay= -aj*(xxi*zeta-zxi*xeta);
        zetaz=  aj*(xxi*yeta-yxi*xeta);

        //  Finally, the vorticity components.
        //
        vort[0]= xiy*wxi+etay*weta+zetay*wzeta - xiz*vxi-etaz*veta-zetaz*vzeta;
        vort[1]= xiz*uxi+etaz*ueta+zetaz*uzeta - xix*wxi-etax*weta-zetax*wzeta;
        vort[2]= xix*vxi+etax*veta+zetax*vzeta - xiy*uxi-etay*ueta-zetay*uzeta;
        idx = i + j*dims[0] + k*ijsize;
        vorticity[idx*3+0] = vort[0];
        vorticity[idx*3+1] = vort[1];
        vorticity[idx*3+2] = vort[2];
        }
      }
    }
}

template <class DataType>
void PLOT3DFunctions<DataType>::ComputePressureGradient(
    int numPts,
    DataType *gradient,
    DataType *Points,
    DataType *Pressure,
    int *dims)
{
  int ijsize;
  int i, j, k, idx, idx2, ii, z;
  double g[3], xp[3], xm[3], pp, pm, factor;
  double xxi, yxi, zxi, pxi;
  double xeta, yeta, zeta, peta;
  double xzeta, yzeta, zzeta, pzeta;
  double aj, xix, xiy, xiz, etax, etay, etaz, zetax, zetay, zetaz;

  ijsize = dims[0]*dims[1];

  for (k=0; k<dims[2]; k++)
    {
    for (j=0; j<dims[1]; j++)
      {
      for (i=0; i<dims[0]; i++)
        {
        //  Xi derivatives.
        if ( dims[0] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            xp[ii] = xm[ii] = 0.0;
            }
          xp[0] = 1.0; pp = pm = 0.0;
          }
        else if ( i == 0 )
          {
          factor = 1.0;
          idx = (i+1) + j*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            }
          pp = Pressure[idx];
          pm = Pressure[idx2];
          }
        else if ( i == (dims[0]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i-1 + j*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            }
          pp = Pressure[idx];
          pm = Pressure[idx2];
          }
        else
          {
          factor = 0.5;
          idx = (i+1) + j*dims[0] + k*ijsize;
          idx2 = (i-1) + j*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            }
          pp = Pressure[idx];
          pm = Pressure[idx2];
          }

        xxi = factor * (xp[0] - xm[0]);
        yxi = factor * (xp[1] - xm[1]);
        zxi = factor * (xp[2] - xm[2]);
        pxi = factor * (pp - pm);

        //  Eta derivatives.
        if ( dims[1] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            xp[ii] = xm[ii] = 0.0;
            }
          xp[1] = 1.0; pp = pm = 0.0;
          }
        else if ( j == 0 )
          {
          factor = 1.0;
          idx = i + (j+1)*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            }
          pp = Pressure[idx];
          pm = Pressure[idx2];
          }
        else if ( j == (dims[1]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i + (j-1)*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            }
          pp = Pressure[idx];
          pm = Pressure[idx2];
          }
        else
          {
          factor = 0.5;
          idx = i + (j+1)*dims[0] + k*ijsize;
          idx2 = i + (j-1)*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            }
          pp = Pressure[idx];
          pm = Pressure[idx2];
          }

        xeta = factor * (xp[0] - xm[0]);
        yeta = factor * (xp[1] - xm[1]);
        zeta = factor * (xp[2] - xm[2]);
        peta = factor * (pp - pm);

        //  Zeta derivatives.
        if ( dims[2] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            xp[ii] = xm[ii] = 0.0;
            }
          xp[2] = 1.0; pp = pm = 0.0;
          }
        else if ( k == 0 )
          {
          factor = 1.0;
          idx = i + j*dims[0] + (k+1)*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            }
          pp = Pressure[idx];
          pm = Pressure[idx2];
          }
        else if ( k == (dims[2]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + (k-1)*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            }
          pp = Pressure[idx];
          pm = Pressure[idx2];
          }
        else
          {
          factor = 0.5;
          idx = i + j*dims[0] + (k+1)*ijsize;
          idx2 = i + j*dims[0] + (k-1)*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            }
          pp = Pressure[idx];
          pm = Pressure[idx2];
          }

        xzeta = factor * (xp[0] - xm[0]);
        yzeta = factor * (xp[1] - xm[1]);
        zzeta = factor * (xp[2] - xm[2]);
        pzeta = factor * (pp - pm);

        //  Now calculate the Jacobian.  Grids occasionally have
        //  singularities, or points where the Jacobian is infinite (the
        //  inverse is zero).  For these cases, we'll set the Jacobian to
        //  zero, which will result in a zero vorticity.
        //
        aj =  xxi*yeta*zzeta+yxi*zeta*xzeta+zxi*xeta*yzeta
              -zxi*yeta*xzeta-yxi*xeta*zzeta-xxi*zeta*yzeta;
        if (aj != 0.0)
          {
          aj = 1. / aj;
          }

        //  Xi metrics.
        xix  =  aj*(yeta*zzeta-zeta*yzeta);
        xiy  = -aj*(xeta*zzeta-zeta*xzeta);
        xiz  =  aj*(xeta*yzeta-yeta*xzeta);

        //  Eta metrics.
        etax = -aj*(yxi*zzeta-zxi*yzeta);
        etay =  aj*(xxi*zzeta-zxi*xzeta);
        etaz = -aj*(xxi*yzeta-yxi*xzeta);

        //  Zeta metrics.
        zetax=  aj*(yxi*zeta-zxi*yeta);
        zetay= -aj*(xxi*zeta-zxi*xeta);
        zetaz=  aj*(xxi*yeta-yxi*xeta);

        //  Finally, the vorticity components.
        g[0]= xix*pxi+etax*peta+zetax*pzeta;
        g[1]= xiy*pxi+etay*peta+zetay*pzeta;
        g[2]= xiz*pxi+etaz*peta+zetaz*pzeta;

        idx = i + j*dims[0] + k*ijsize;
        gradient[3*idx+0] = g[0];
        gradient[3*idx+1] = g[1];
        gradient[3*idx+2] = g[2];
        }
      }
    }
}

template <class DataType>
void PLOT3DFunctions<DataType>::ComputePressureCoefficient(
    int numPts,
    DataType *pressure_coeff,
    DataType *Density,
    DataType *Momentum,
    DataType *Energy,
    DataType *Gamma,
    DataType gi,
    DataType fsm)
{
  double m[3], e, g, u, v, w, v2, p, d, rr, pi, den;

  den = .5*(double)fsm*(double)fsm;
  for (int i=0; i < numPts; i++)
    {
    d = Density[i];
    d = (d != 0.0 ? d : 1.0);
    m[0] = Momentum[3*i+0];
    m[1] = Momentum[3*i+1];
    m[2] = Momentum[3*i+2];
    e = Energy[i];
    g = Gamma[i];
    pi = 1.0 / (double)gi;
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    p = (g-1.) * (e - 0.5 * d * v2);
    pressure_coeff[i] = (p - pi)/den;
    }
}

template <class DataType>
void PLOT3DFunctions<DataType>::ComputeMachNumber(
    int numPts,
    DataType *machnumber,
    DataType *Density,
    DataType *Momentum,
    DataType *Energy,
    DataType *Gamma)
{
  double m[3], e, g, u, v, w, v2, a2, d, rr;

  //  Compute the mach number
  //
  for (int i=0; i < numPts; i++)
    {
    d = Density[i];
    d = (d != 0.0 ? d : 1.0);
    m[0] = Momentum[3*i+0];
    m[1] = Momentum[3*i+1];
    m[2] = Momentum[3*i+2];
    e = Energy[i];
    g = Gamma[i];
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    a2 = g * (g-1.) * (e * rr - .5*v2);
    machnumber[i] = sqrt(v2/a2);
  }
}

template <class DataType>
void PLOT3DFunctions<DataType>::ComputeSoundSpeed(
    int numPts,
    DataType *soundspeed,
    DataType *Density,
    DataType *Momentum,
    DataType *Energy,
    DataType *Gamma)
{
  double m[3], e, u, v, w, v2, p, d, g, rr;

  //  Compute sound speed
  //
  for (int i=0; i < numPts; i++)
    {
    d = Density[i];
    d = (d != 0.0 ? d : 1.0);
    m[0] = Momentum[3*i+0];
    m[1] = Momentum[3*i+1];
    m[2] = Momentum[3*i+2];
    e = Energy[i];
    g = Gamma[i];
    rr = 1.0 / d;
    u = m[0] * rr;
    v = m[1] * rr;
    w = m[2] * rr;
    v2 = u*u + v*v + w*w;
    p = (g-1.) * (e - 0.5 * d * v2);
    soundspeed[i]  = sqrt(g*p*rr);
  }
}

template <class DataType>
void PLOT3DFunctions<DataType>::ComputeStrainRate(
    int numPts,
    DataType *strainRate,
    DataType *Points,
    DataType *Velocity,
    int *dims)
{
  int ijsize;
  int i, j, k, idx, idx2, ii, z;
  double stRate[3], xp[3], xm[3], vp[3], vm[3], factor;
  double xxi, yxi, zxi, uxi, vxi, wxi;
  double xeta, yeta, zeta, ueta, veta, weta;
  double xzeta, yzeta, zzeta, uzeta, vzeta, wzeta;
  double aj, xix, xiy, xiz, etax, etay, etaz, zetax, zetay, zetaz;

  ijsize = dims[0]*dims[1];

  for (k=0; k<dims[2]; k++)
    {
    for (j=0; j<dims[1]; j++)
      {
      for (i=0; i<dims[0]; i++)
        {
        //  Xi derivatives.
        if ( dims[0] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            vp[ii] = vm[ii] = xp[ii] = xm[ii] = 0.0;
            }
          xp[0] = 1.0;
          }
        else if ( i == 0 )
          {
          factor = 1.0;
          idx = (i+1) + j*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }
        else if ( i == (dims[0]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i-1 + j*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }
        else
          {
          factor = 0.5;
          idx = (i+1) + j*dims[0] + k*ijsize;
          idx2 = (i-1) + j*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }

        xxi = factor * (xp[0] - xm[0]);
        yxi = factor * (xp[1] - xm[1]);
        zxi = factor * (xp[2] - xm[2]);
        uxi = factor * (vp[0] - vm[0]);
        vxi = factor * (vp[1] - vm[1]);
        wxi = factor * (vp[2] - vm[2]);

        //  Eta derivatives.
        if ( dims[1] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            vp[ii] = vm[ii] = xp[ii] = xm[ii] = 0.0;
            }
          xp[1] = 1.0;
          }
        else if ( j == 0 )
          {
          factor = 1.0;
          idx = i + (j+1)*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }
        else if ( j == (dims[1]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i + (j-1)*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }
        else
          {
          factor = 0.5;
          idx = i + (j+1)*dims[0] + k*ijsize;
          idx2 = i + (j-1)*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }


        xeta = factor * (xp[0] - xm[0]);
        yeta = factor * (xp[1] - xm[1]);
        zeta = factor * (xp[2] - xm[2]);
        ueta = factor * (vp[0] - vm[0]);
        veta = factor * (vp[1] - vm[1]);
        weta = factor * (vp[2] - vm[2]);

        //  Zeta derivatives.
        if ( dims[2] == 1 ) // 2D in this direction
          {
          factor = 1.0;
          for (ii=0; ii<3; ii++)
            {
            vp[ii] = vm[ii] = xp[ii] = xm[ii] = 0.0;
            }
          xp[2] = 1.0;
          }
        else if ( k == 0 )
          {
          factor = 1.0;
          idx = i + j*dims[0] + (k+1)*ijsize;
          idx2 = i + j*dims[0] + k*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }
        else if ( k == (dims[2]-1) )
          {
          factor = 1.0;
          idx = i + j*dims[0] + k*ijsize;
          idx2 = i + j*dims[0] + (k-1)*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }
        else
          {
          factor = 0.5;
          idx = i + j*dims[0] + (k+1)*ijsize;
          idx2 = i + j*dims[0] + (k-1)*ijsize;
          for (z = 0; z < 3; ++z)
            {
            xp[z] = Points[3*idx+z];
            xm[z] = Points[3*idx2+z];
            vp[z] = Velocity[3*idx+z];
            vm[z] = Velocity[3*idx2+z];
            }
          }

        xzeta = factor * (xp[0] - xm[0]);
        yzeta = factor * (xp[1] - xm[1]);
        zzeta = factor * (xp[2] - xm[2]);
        uzeta = factor * (vp[0] - vm[0]);
        vzeta = factor * (vp[1] - vm[1]);
        wzeta = factor * (vp[2] - vm[2]);

        // Now calculate the Jacobian.  Grids occasionally have
        // singularities, or points where the Jacobian is infinite (the
        // inverse is zero).  For these cases, we'll set the Jacobian to
        // zero, which will result in a zero vorticity.
        //
        aj =  xxi*yeta*zzeta+yxi*zeta*xzeta+zxi*xeta*yzeta
              -zxi*yeta*xzeta-yxi*xeta*zzeta-xxi*zeta*yzeta;
        if (aj != 0.0)
          {
          aj = 1. / aj;
          }

        //  Xi metrics.
        xix  =  aj*(yeta*zzeta-zeta*yzeta);
        xiy  = -aj*(xeta*zzeta-zeta*xzeta);
        xiz  =  aj*(xeta*yzeta-yeta*xzeta);

        //  Eta metrics.
        etax = -aj*(yxi*zzeta-zxi*yzeta);
        etay =  aj*(xxi*zzeta-zxi*xzeta);
        etaz = -aj*(xxi*yzeta-yxi*xzeta);

        //  Zeta metrics.
        zetax=  aj*(yxi*zeta-zxi*yeta);
        zetay= -aj*(xxi*zeta-zxi*xeta);
        zetaz=  aj*(xxi*yeta-yxi*xeta);

        //  Finally, the strain rate components.
        //
        stRate[0] = xix*uxi+etax*ueta+zetax*uzeta;
        stRate[1] = xiy*vxi+etay*veta+zetay*vzeta;
        stRate[2] = xiz*wxi+etaz*weta+zetaz*wzeta;
        idx = i + j*dims[0] + k*ijsize;
        strainRate[3*idx+0] = stRate[0];
        strainRate[3*idx+1] = stRate[1];
        strainRate[3*idx+2] = stRate[2];
        }
      }
    }
}


template class PLOT3DFunctions<float>;
template class PLOT3DFunctions<double>;

