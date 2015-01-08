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
//                                avtIVPField.h                              //
// ************************************************************************* //

#ifndef AVT_IVPFIELD_H
#define AVT_IVPFIELD_H

#include <stdexcept>
#include <avtVector.h>
#include <ivp_exports.h>

// ****************************************************************************
//  Class: avtIVPField
//
//  Purpose:
//      avtIVPField is a base class for all manners of vector fields. 
//      Deriving from it should allow an adaptation of many different vector 
//      field types for the use of streamlines/IVP solutions by wrapping 
//      existing interpolation facilities.
//
//      The IVP right-hand side is made accessible to an IVP solver by means of 
//      the avtIVPField class, allowing the IVP solver to query points of the 
//      given vector field. 
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar 10 12:41:11 EDT 2009
//   Add GetValidTimeRange.
//
//   Dave Pugmire, Mon Jun 8 2009, 11:44:01 EDT 2009
//   Added ComputeScalarVariable, HasGhostZones and GetExtents methods.
//
//   Dave Pugmire, Tue Dec  1 11:50:18 EST 2009
//   Switch from avtVec to avtVector.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Generalize the compute scalar variable.
//
//   Christoph Garth, July 13 16:49:12 PDT 2010
//   Compute scalars by index instead of by name.
//
//   Hank Childs, Sun Dec  5 10:18:13 PST 2010
//   Add a boolean for whether or not the velocity field is instantaneous.
//
//   Dave Pugmire, Fri Jun  1 10:03:14 EDT 2012
//   Add Classification enum for IsInside() method.
//
//   Dave Pugmire, Wed Jun 13 17:18:24 EDT 2012
//   Added avtIVPField::Result.
//
// ****************************************************************************

class IVP_API avtIVPField
{
  public:
    enum Result
    {
        OK = 0,
        OUTSIDE_SPATIAL,
        OUTSIDE_TEMPORAL,
        OUTSIDE_BOTH,
        FAIL
    };

                         avtIVPField() : order(1) {}
    virtual             ~avtIVPField() {}

    virtual Result       operator()(const double& t, 
                                    const avtVector& x,
                                    avtVector& retV) const = 0;

    virtual Result       operator()(const double& t, 
                                    const avtVector& x, 
                                    const avtVector& v,
                                    avtVector& retV) const = 0;

    virtual avtVector    ConvertToCartesian(const avtVector& pt) const = 0;
    virtual avtVector    ConvertToCylindrical(const avtVector& pt) const = 0;

    virtual double       ComputeVorticity(const double& t, 
                                          const avtVector& x ) const = 0;
    virtual double       ComputeScalarVariable(unsigned char index,
                                               const double& t,
                                               const avtVector& x) const = 0;

    virtual void         SetScalarVariable(unsigned char index, 
                                           const std::string& name ) = 0;

    virtual Result       IsInside(const double &t, const avtVector &x) const = 0;

    virtual void         GetTimeRange( double range[2] ) const = 0;
    virtual void         GetExtents( double  extents[6] ) const = 0;
    virtual bool         VelocityIsInstantaneous(void) { return true; }

    virtual void         SetOrder( unsigned int val ) { order = val; }
    virtual unsigned int GetOrder() { return order; }

 protected:
    unsigned int order;
};

// ostream operators for avtICStatus
inline std::ostream& operator<<(std::ostream& out, 
                                avtIVPField::Result res)
{
    if (res == avtIVPField::OK)
        out<<"OK";
    else if (res == avtIVPField::OUTSIDE_SPATIAL)
        out<<"OUTSIDE_SPATIAL";
    else if (res == avtIVPField::OUTSIDE_TEMPORAL)
        out<<"OUTSIDE_TEMPORAL";
    else if (res == avtIVPField::OUTSIDE_BOTH)
        out<<"OUTSIDE_BOTH";
    else if (res == avtIVPField::FAIL)
        out<<"FAIL"<<endl;
    return out;
}


#endif
