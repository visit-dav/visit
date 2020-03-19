// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  avtPseudocolorGeometryFilter.h
// ************************************************************************* //

#ifndef AVT_PSEUDOCOLOR_GEOMETRY_FILTER_H
#define AVT_PSEUDOCOLOR_GEOMETRY_FILTER_H

#include <avtSIMODataTreeIterator.h>

#include <PseudocolorAttributes.h>

class vtkPolyData;

// ****************************************************************************
//  Class: avtPseudocolorGeometryFilter
//
//  Purpose:  Separates vertex cells into separate output datset, applies
//            tubes to lines and/or glyphs to line endpoints if necessary
//            based on the atts settings.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 20, 2019
//
//  Modifications:
//
// ****************************************************************************

class avtPseudocolorGeometryFilter : public avtSIMODataTreeIterator
{
  public:
                            avtPseudocolorGeometryFilter();
    virtual                ~avtPseudocolorGeometryFilter();

    virtual const char     *GetType(void)
                              { return "avtPseudocolorGeometryFilter"; }
    virtual const char     *GetDescription(void)
                              { return "Applying Pseudocolor Geo filter"; }
    void                    SetPlotAtts(const PseudocolorAttributes *);

  protected:

    virtual avtDataTree_p   ExecuteDataTree(avtDataRepresentation *);
    virtual void            UpdateDataObjectInfo(void);
    virtual void            PostExecute(void);

  private:
    PseudocolorAttributes   plotAtts;

    void                    ProcessPoints(const vtkDataSet *, vtkDataSet *&,
                                          bool &, bool &);
    void                    ProcessLines(const vtkDataSet *, vtkPolyData *&,
                                         bool &);

    void                    AddTubes(vtkPolyData *, vtkPolyData *,
                                     double);
    void                    AddRibbons(vtkPolyData *, vtkPolyData *,
                                     double);
    void                    AddEndPoints(vtkPolyData *, vtkPolyData *,
                                     double);
};


#endif


