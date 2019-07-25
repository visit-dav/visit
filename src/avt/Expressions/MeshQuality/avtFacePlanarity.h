// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtFacePlanarity.h                             //
// ************************************************************************* //

#ifndef AVT_FACE_PLANARITY_H
#define AVT_FACE_PLANARITY_H

#include <avtSingleInputExpressionFilter.h>

class     vtkCell;


// ****************************************************************************
//  Class: avtFacePlanarity
//
//  Purpose: Calculates either relative or absolute face planarity for a cell.
//  Face planarity is defined ONLY for cells that have at least one face that
//  is non-simplex (e.g. non-triangle). That means wedge, pyramid and hex
//  cells. For a given face, a face planarity measure of zero means all the
//  points on the face lie on the same plane. This is true by definition for
//  triangular faces. For quadrilateral faces, the distance between the 4th
//  node and the plane defined by the first 3 nodes represents the face
//  planarity measure. If this distance is divided by the average of the 
//  lengths of the 4 edges, it is a relative planarity measure for the face. 
//  Finally, the face planarity measure for a whole cell consisting
//  of potentially several non-triangular faces is taken to be the maximum
//  over its faces.
//
//  Programmer: Mark C. Miller
//  Created:    Wed Jun  3 12:34:41 PDT 2009
//
// ****************************************************************************

class EXPRESSION_API avtFacePlanarity : public avtSingleInputExpressionFilter
{
  public:
                                avtFacePlanarity();

    virtual const char         *GetType(void) { return "avtFacePlanarity"; };
    virtual const char         *GetDescription(void)
                                    { return "Calculating face planarities"; };

    void                        SetTakeRelative(bool tr) { takeRel = tr; };
    
  protected:
    bool                        takeRel;

    virtual vtkDataArray       *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool                IsPointVariable(void)  { return false; };
    virtual int                 GetVariableDimension()   { return 1; };
};


#endif


