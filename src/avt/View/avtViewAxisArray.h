// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtViewAxisArray.h                          //
// ************************************************************************* //

#ifndef AVT_VIEW_AXIS_ARRAY_H
#define AVT_VIEW_AXIS_ARRAY_H
#include <view_exports.h>
#include <enumtypes.h>

struct avtViewInfo;
class ViewAxisArrayAttributes;

// ****************************************************************************
//  Class: avtViewAxisArray
//
//  Purpose:
//    Contains the information for a axis array view.
//
//  Programmer: Jeremy Meredith 
//  Creation:   January 28, 2008
//
//  Modifications:
//    Eric Brugger, Mon Nov  5 15:12:30 PST 2012
//    I added the ability to display the parallel axes either horizontally
//    or vertically.
//
// ****************************************************************************

struct AVTVIEW_API avtViewAxisArray
{
    double    viewport[4];
    double    domain[2];
    double    range[2];

  public:
                          avtViewAxisArray();
    avtViewAxisArray     &operator=(const avtViewAxisArray &);
    bool                  operator==(const avtViewAxisArray &);
    void                  SetToDefault(void);
    void                  SetViewInfoFromView(avtViewInfo &, bool, int *);
    void                  SetViewport(double *newViewport);

    void                  GetViewport(double *) const;
    double                GetScaleFactor(int *);

    void                  SetFromViewAxisArrayAttributes(
                                           const ViewAxisArrayAttributes *);
    void                  SetToViewAxisArrayAttributes(
                                           ViewAxisArrayAttributes *) const;

  protected:
    double                viewScale;

    void                  CheckAndCorrectDomainRange();
};


#endif

