// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtIVPVTKEdgeField.h                         //
// ************************************************************************* //

#ifndef AVT_IVPVTKEDGEFIELD_H
#define AVT_IVPVTKEDGEFIELD_H

#include <vtkCell.h>

#include <avtIVPVTKField.h>
#include <avtCellLocatorRectEdge.h>
#include <ivp_exports.h>

#include <vector>

class vtkDataSet;
class vtkDataArray;

// ****************************************************************************
//  Class:  avtIVPVTKEdgeField
//
//  Purpose:
//    A wrapper class to allow the use of vtkDataSets as IVP fields for 
//    integral curve integration. Uses vtkInterpolatedVelocityField on top of 
//    the supplied vtkDataSet. Fields are allowed to have offsets, i.e. 
//    each component can have a different stagger.
//
//  Programmer:  Alexander Pletzer
//  Creation:    Sun Dec  1 19:32:05 MST 2013
//
//  Modifications:
//
//
// ****************************************************************************

class IVP_API avtIVPVTKEdgeField: public avtIVPVTKField
{
  public:
    avtIVPVTKEdgeField( vtkDataSet* dataset, 
                          avtCellLocator* locator );
    ~avtIVPVTKEdgeField();

    virtual avtVector operator()( const double &t, const avtVector &p ) const;
    virtual Result    operator()(const double& t,
                                 const avtVector &pt,
                                 avtVector &retV) const;

    virtual bool      FindValue(vtkDataArray *vectorData, avtVector &vel, 
                                double t, const avtVector &p) const;

  protected:
    avtCellLocatorRectEdge *locRE;

  private:

};

#endif
