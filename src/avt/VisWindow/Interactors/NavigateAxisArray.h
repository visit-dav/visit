// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           NavigateAxisArray.h                             //
// ************************************************************************* //

#ifndef NAVIGATE_AXIS_ARRAY_H
#define NAVIGATE_AXIS_ARRAY_H
#include <viswindow_exports.h>


#include <VisitInteractor.h>


class VisWindowInteractorProxy;


// ****************************************************************************
//  Class: NavigateAxisArray
//
//  Purpose:
//      Defines what Visit's AxisArray Navigation interaction should look like.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 29, 2008
//
//  Modifications:
//    Jeremy Meredith, Thu Feb  7 17:58:11 EST 2008
//    Added support for toggling horizontal snap-to-grid.
//
//    Eric Brugger, Tue Dec  9 14:48:50 PST 2008
//    Added an axis orientation, which interchanges the horizontal and
//    vertical zooming.
//
//    Eric Brugger, Mon Nov  5 15:43:16 PST 2012
//    I added the ability to display the parallel axes either horizontally
//    or vertically.
//
// ****************************************************************************

class VISWINDOW_API NavigateAxisArray : public VisitInteractor
{
  public:
                        NavigateAxisArray(VisWindowInteractorProxy &);
 
    virtual void        OnTimer(void);

    virtual void        StartLeftButtonAction();
    virtual void        EndLeftButtonAction();
    virtual void        StartMiddleButtonAction();
    virtual void        EndMiddleButtonAction();
    virtual void        OnMouseWheelForward();
    virtual void        OnMouseWheelBackward();

    enum Orientation
    {
        Horizontal,
        Vertical
    };
    void                SetAxisOrientation(const Orientation orientation);
    void                SetDomainOrientation(const Orientation orientation);

  private:
    void                PanCamera(const int x, const int y);
    void                ZoomCamera(const int x, const int y);
    void                ZoomHorizontal(double f);
    void                ZoomHorizontalFixed(double f);
    void                ZoomVertical(double f);
    void                ZoomVerticalFixed(double f);

    bool                shouldSnap;
    bool                shiftKeyDown;
    bool                controlKeyDown;

    Orientation         axisOrientation;
    Orientation         domainOrientation;
};


#endif


