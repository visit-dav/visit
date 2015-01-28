/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
//                         avtIVPNek5000TimeVaryingField.h                   //
// ************************************************************************* //

#ifndef AVT_IVPNEK5000TIMEVARYINGFIELD_H
#define AVT_IVPNEK5000TIMEVARYINGFIELD_H

#include <avtIVPField.h>
#include <avtCellLocator.h>
#include <ivp_exports.h>

class vtkDataSet;
class vtkDataArray;

struct findpts_local_data;

// ****************************************************************************
//  Class:  avtIVPNek5000TimeVaryingField
//
//  Purpose:
//    A wrapper class to allow the use of vtkDataSets as IVP fields for 
//    streamline integration. Uses vtkInterpolatedVelocityField on top of 
//    the supplied vtkDataSet. 
//
//  Programmer:  Christoph Garth
//  Creation:    Sun Feb 24 19:16:09 PST 2008
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Added GetValidTimeRange function.
//
//   Hank Childs, Thu Apr  2 16:40:08 PDT 2009
//   Use vtkVisItInterpolatedVelocityField, not vtktInterpolatedVelocityField.
//
//   Dave Pugmire, Mon Jun 8 2009, 11:44:01 EDT 2009
//   Added ComputeScalarVariable, HasGhostZones and GetExtents methods.
//
//    Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//    Switch from avtVec to avtVector.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Generalize the compute scalar variable.
//
//   Christoph Garth, Fri Jul 9, 10:10:22 PDT 2010
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPNek5000TimeVaryingField.
//
//   Christoph Garth, July 13 16:49:12 PDT 2010
//   Compute scalars by index instead of by name.
//
//   Hank Childs, Sun Dec  5 10:18:13 PST 2010
//   Indicate the velocity is not instantaneous.
//
//   Christoph Garth, Tue Mar 6 16:38:00 PDT 2012
//   Moved ghost data handling into cell locator and changed IsInside()
//   to only consider non-ghost cells.
//
// ****************************************************************************

class IVP_API avtIVPNek5000TimeVaryingField : public avtIVPField
{
  public:
    avtIVPNek5000TimeVaryingField( vtkDataSet* dataset, avtCellLocator* locator,
                               double t0, double t1 );
    ~avtIVPNek5000TimeVaryingField();

    // avtIVPField interface
    virtual Result    operator()(const double& t,
                                 const avtVector &pt,
                                       avtVector &retV) const;

    virtual Result    operator()(const double& t,
                                 const avtVector &pt,
                                 const avtVector &vel,
                                       avtVector &retV) const
    { return FAIL; }

    virtual avtVector ConvertToCartesian(const avtVector& pt) const;
    virtual avtVector ConvertToCylindrical(const avtVector& pt) const;

    virtual double    ComputeVorticity(const double& t, const avtVector &pt) const;

    virtual double    ComputeScalarVariable(unsigned char index,
                                            const double& t,
                                            const avtVector& x) const;

    virtual void      SetScalarVariable( unsigned char index, 
                                         const std::string& name );

    Result         IsInside( const double& t, const avtVector &pt ) const;
    unsigned int   GetDimension() const;

    virtual void   GetExtents( double extents[6] ) const;
    virtual void   GetTimeRange( double range[2] ) const;
    virtual bool   VelocityIsInstantaneous(void) { return false; };

    static const char* NextTimePrefix;

  protected:

    Result         FindCell( const double& t, const avtVector& p ) const;

    vtkDataSet*            ds;
    avtCellLocator*        loc;

    vtkDataArray*          velData[2];
    bool                   velCellBased;
    std::vector<std::string>         sclDataName;
    vtkDataArray*          sclData[2][256];
    bool                   sclCellBased[256];
    double                 t0, t1, dt;

    mutable avtVector               lastPos;
    mutable vtkIdType               lastCell;
    mutable avtInterpolationWeights lastWeights;

    // Storage for the Nek5000 mesh and the vector.
    double *nek_pts[2][3], *nek_vec[2][3];

    // Variables needed for Nek5000 find points
    mutable struct findpts_local_data *nek_fld[2];
};

#endif
