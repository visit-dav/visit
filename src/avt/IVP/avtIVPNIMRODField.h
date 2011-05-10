/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
//                            avtIVPNIMRODField.h                             //
// ************************************************************************* //

#ifndef AVT_IVP_NIMROD_FIELD_H
#define AVT_IVP_NIMROD_FIELD_H

#include "avtIVPVTKField.h"
#include <avtMatrix.h>

#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ivp_exports.h>

typedef avtVector vec3;
typedef avtMatrix mat3;

// ****************************************************************************
//  Class:  avtIVPNIMRODField
//
//  Purpose:
//    A wrapper class to allow the use of vtkDataSets as IVP fields for 
//    streamline integration. Uses vtkInterpolatedVelocityField on top of 
//    the supplied vtkDataSet. 
//
//  Programmer:  Allen Sanderson
//  Creation:    20 Nov 2009
//
// ****************************************************************************

class IVP_API avtIVPNIMRODField: public avtIVPVTKField
{
 protected:
  double lagrange_nodes[6][6];
  
 public:
  avtIVPNIMRODField( vtkDataSet* ds, avtCellLocator* loc ); 
  avtIVPNIMRODField( unsigned int nRad,
                     unsigned int nTheta,
                     unsigned int nPhi,
                     float *grid_fourier_series,
                     float *data_fourier_series );

  ~avtIVPNIMRODField();

  avtVector operator()( const double &t, const avtVector &p ) const;

  avtVector ConvertToCartesian(const avtVector& pt) const;
  avtVector ConvertToCylindrical(const avtVector& pt) const;

 protected:
  template< class type >
    type* SetDataPointer( vtkDataSet *ds,
                          const type var,
                          const char* varname,
                          const int ncomponents );

  void lagrange_weights( unsigned int DEG, const double s, 
                         double* w = NULL, double *d = NULL ) const;

  void fourier_weights( unsigned int N, const double t, 
                        double* w, double* d = NULL ) const;

  void interpolate( double rad, double theta, double phi,
                    vec3* P, mat3* DRV ) const;

 public: 
  // Variables read as part of the mesh.
  float *grid_fourier_series;
  float *data_fourier_series;

  unsigned int Nrad, Ntheta, Nphi;

  // variables read from header attributes.
  unsigned int Drad;   // = 2;
  unsigned int Dtheta; // = 2;
};

#endif
