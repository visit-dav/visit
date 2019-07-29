// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtMultiresFilter.h                            //
// ************************************************************************* //

#ifndef AVT_MULTIRES_FILTER_H
#define AVT_MULTIRES_FILTER_H

#include <filters_exports.h>

#include <avtDatasetToDatasetFilter.h>

class vtkDataSet;

class avtMultiresSelection;

// ****************************************************************************
//  Class: avtMultiresFilter
//
//  Purpose:
//    Modify the input to only serve up a subset of the chunks based on the
//    view frustum and the smallest cell size.
//
//  Programmer: Eric Brugger
//  Creation:   Tue Oct 25 14:04:58 PDT 2011
//
//  Modifications:
//    Eric Brugger, Fri Dec 20 11:50:18 PST 2013
//    Add support for doing multi resolution data selections.
//
//    Eric Brugger, Thu Jan  2 15:15:54 PST 2014
//    Add support for 3d multi resolution data selections.
//
//    Eric Brugger, Wed Jan  8 16:58:34 PST 2014
//    I added a ViewArea to the multi resolution data selection since the
//    view frustum was insufficient in 3d.
//
// ****************************************************************************

class AVTFILTERS_API avtMultiresFilter : public avtDatasetToDatasetFilter
{
  public:
                           avtMultiresFilter(double *, double *, double *,
                               double *, int *, double, double,
                               double *, double *, double);
    virtual               ~avtMultiresFilter();

    virtual const char    *GetType(void)  {return "avtMultiresFilter";};
    virtual const char    *GetDescription(void) {return "Multires";};

  protected:
    int                    nDims;
    double                 transform2D[16];
    double                 transform3D[16];
    double                 transform[16];
    double                 viewport2D[6];
    double                 viewport3D[6];
    double                 viewport[6];
    int                    windowSize[2];
    double                 viewArea2D;
    double                 viewArea3D;
    double                 viewArea;
    double                 desiredExtents2D[6];
    double                 desiredExtents3D[6];
    double                 desiredExtents[6];
    double                 desiredCellArea;
    double                 actualCellArea;

    int                    selID;

    virtual void           Execute(void);

    virtual avtContract_p  ModifyContract(avtContract_p contract);
};


#endif
