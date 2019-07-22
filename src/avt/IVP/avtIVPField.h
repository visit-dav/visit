// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//      field types for the use of integral curves/IVP solutions by wrapping 
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

    avtIVPField() : order(1), directionless(false),
      hasPeriodicBoundaries(false),
      periodic_boundary_x(0), periodic_boundary_y(0), periodic_boundary_z(0) {}

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

    virtual void         SetDirectionless( bool val ) { directionless = val; }
    virtual bool         GetDirectionless() { return directionless; }

    virtual bool         HasPeriodicBoundaries() const { return false; }
    virtual void         GetBoundaries( double& x,
                                        double& y,
                                        double& z) const { x = y = z = 0; }

 protected:
    unsigned int order;
    bool directionless, hasPeriodicBoundaries;
    double periodic_boundary_x, periodic_boundary_y, periodic_boundary_z;
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
