// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtIVPVTKOffsetField.h                       //
// ************************************************************************* //

#ifndef AVT_IVPVTKOFFSETFIELD_H
#define AVT_IVPVTKOFFSETFIELD_H

#include <vtkCell.h>

#include <avtIVPVTKField.h>
#include <avtCellLocator.h>
#include <ivp_exports.h>

#include <vector>

class vtkDataSet;
class vtkDataArray;

// ****************************************************************************
//  Class:  avtIVPVTKOffsetField
//
//  Purpose:
//    A wrapper class to allow the use of vtkDataSets as IVP fields for 
//    integral curve integration. Uses vtkInterpolatedVelocityField on top of 
//    the supplied vtkDataSet. Fields are allowed to have offsets, i.e. 
//    each component can have a different stagger.
//
//  Programmer:  Alexander Pletzer
//  Creation:    Thu Mar  1 12:01:37 MST 2012
//
//  Modifications:
//
//
// ****************************************************************************

class IVP_API avtIVPVTKOffsetField: public avtIVPVTKField
{
  public:
    avtIVPVTKOffsetField( vtkDataSet* dataset, 
                          avtCellLocator* locator );
    ~avtIVPVTKOffsetField();

    virtual avtVector operator()( const double &t, const avtVector &p ) const;
    virtual Result    operator()(const double& t,
                                 const avtVector &pt,
                                 avtVector &retV) const;

    virtual bool      FindValue(vtkDataArray *vectorData, avtVector &vel) const;

    void SetNodeOffsets( const std::vector<avtVector>& offsets );

  protected:

    avtVector GetPositionCorrection( size_t compIndex ) const;

    std::vector<avtVector> nodeOffsets;

  private:
};

#endif
