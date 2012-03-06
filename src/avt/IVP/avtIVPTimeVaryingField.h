/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//                         avtIVPTimeVaryingField.h                        //
// ************************************************************************* //

#ifndef AVT_IVPTIMEVARYINGFIELD_H
#define AVT_IVPTIMEVARYINGFIELD_H

#include <stdexcept>
#include <avtIVPField.h>
#include <ivp_exports.h>

// ****************************************************************************
//  Class: avtIVPTimeVaryingField
//
//  Purpose:
//      avtIVPTimeVaryingField combines two avtIVPFieldInstances,
//      corresponding to two different time steps, and performs
//      linear interpolation between them over the specified time interval.
//
//  Programmer: Christoph Garth
//  Creation:   July 22, 2010
//
// ****************************************************************************

class IVP_API avtIVPTimeVaryingField: public avtIVPField
{
public:

    avtIVPTimeVaryingField( double t0, avtIVPField* f0,
                              double t1, avtIVPField* f1 ) :
        time0(t0), time1(t1), field0(f0), field1(f1)
    {
    }

    ~avtIVPTimeVaryingField()
    {
        delete field0;
        delete field1;
    }

    virtual Result       operator()( const double& t, 
                                  const avtVector& x,
                                        avtVector& retV ) const 
    {
        if( t < time0 || t > time1 )
            return( OUTSIDE_TIME_FRAME );

        double s = (t - time0) / (time1 - time0);
        Result result;
        avtVector v0, v1;

        if( (result = (*field0)(t, x, v0)) == OK )
        {
            if( (result = (*field1)(t, x, v1)) == OK )
            {
                retV = ((1.0 - s) * v0) + (s * v1);
            }
        }
        return( result );
    }

    virtual double       ComputeVorticity(const double& t, 
                                          const avtVector& x ) const
    {
        return 0.0;
    }

    virtual double       ComputeScalarVariable(unsigned char index,
                                               const double& t,
                                               const avtVector& x)
    {
        return 0.0;
    }

    virtual void         SetScalarVariable( unsigned char index, 
                                            const std::string& name )
    {
        field0->SetScalarVariable( index, name );
        field1->SetScalarVariable( index, name );
    }

    virtual bool         IsInside(const double& t, 
                                  const avtVector& x) const
    {
        return 
            t >= time0 && 
            t <= time1 && 
            field0->IsInside(t,x) && 
            field1->IsInside(t,x);
    }

    virtual unsigned int GetDimension() const 
    {
        return 3;
    }

    virtual bool         GetValidTimeRange(double range[]) const
    {
        range[0] = time0;
        range[1] = time1;
        return true;
    }

    virtual bool         HasGhostZones() const
    {
        return field0->HasGhostZones() && field1->HasGhostZones();
    }

    virtual void         GetExtents(double *extents) const
    {
        // This should not happen. 
        // We had abort here, but the svn does not want us to use it
        // unless there is debug around it.
#ifdef DEBUG
        abort();
#endif
    }

protected:

    avtIVPField* field0;
    avtIVPField* field1;

    double time0;
    double time1;
};

#endif


