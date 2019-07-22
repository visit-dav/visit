// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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


