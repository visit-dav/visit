// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtIVPVTKField.h                             //
// ************************************************************************* //

#ifndef AVT_IVPVTKFIELD_H
#define AVT_IVPVTKFIELD_H

#include <avtIVPField.h>
#include <avtCellLocator.h>
#include <ivp_exports.h>

class vtkDataSet;
class vtkDataArray;

// ****************************************************************************
//  Class:  avtIVPVTKField
//
//  Purpose:
//    A wrapper class to allow the use of vtkDataSets as IVP fields for 
//    integral curve integration. Uses vtkInterpolatedVelocityField on top of 
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
//   Incorporate vtkVisItInterpolatedVelocityField in avtIVPVTKField.
//
//   Christoph Garth, July 13 16:49:12 PDT 2010
//   Compute scalars by index instead of by name.
//
//   Dave Pugmire, Mon Feb  7 13:46:56 EST 2011
//   Fix ghost mask for ghost cell integration.
//
//   Christoph Garth, Tue Mar 6 16:38:00 PDT 2012
//   Moved ghost data handling into cell locator and changed IsInside()
//   to only consider non-ghost cells.
//
// ****************************************************************************

class IVP_API avtIVPVTKField: public avtIVPField
{
  public:
    avtIVPVTKField( vtkDataSet* dataset, avtCellLocator* locator );
    ~avtIVPVTKField();

    // avtIVPField interface
    virtual Result    operator()(const double& t,
                                 const avtVector &pt,
                                       avtVector &retV) const;

    virtual Result    operator()(const double& t,
                                 const avtVector &pt,
                                 const avtVector &vel,
                                       avtVector &retV) const
    { return FAIL; }

    virtual bool      FindValue(vtkDataArray *vectorData, avtVector &vel) const;
    virtual bool      FindValue(vtkDataArray *vectorData, avtVector &vel,
                                double t, const avtVector &p) const 
                                { return FindValue(vectorData, vel); }
    virtual avtVector ConvertToCartesian(const avtVector& pt) const;
    virtual avtVector ConvertToCylindrical(const avtVector& pt) const;

    virtual double    ComputeVorticity(const double& t, const avtVector &pt) const;

    virtual double    ComputeScalarVariable(unsigned char index,
                                            const double& t,
                                            const avtVector& x) const;

    virtual void      SetScalarVariable( unsigned char index, 
                                         const std::string& name );

    virtual Result    IsInside( const double& t, const avtVector &pt ) const;
    unsigned int      GetDimension() const {return 3;}
    virtual void      GetExtents( double extents[6] ) const;
    virtual void      GetTimeRange( double range[2] ) const;

    virtual bool      HasPeriodicBoundaries() const;
    virtual void      GetBoundaries( double& x, double& y, double& z) const;

  protected:

    Result             FindCell( const double& t, const avtVector& p ) const;

    vtkDataSet*        ds;
    avtCellLocator*    loc;

    vtkDataArray*      velData;
    bool               velCellBased;
    vtkDataArray*      sclData[256];
    bool               sclCellBased[256];

    mutable avtVector               lastPos;
    mutable vtkIdType               lastCell;
    mutable avtInterpolationWeights lastWeights;

  private:
    static const unsigned char ghostMask;
};

#endif
