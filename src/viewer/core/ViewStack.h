// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEW_STACK_H
#define VIEW_STACK_H
#include <viewercore_exports.h>
#include <avtViewCurve.h>
#include <avtView2D.h>
#include <avtView3D.h>
#include <avtViewAxisArray.h>

#define VSTACK_SIZE 15

// ****************************************************************************
// Class: ViewStack
//
// Purpose:
//   Contains the stacks that allow us to stack different view types.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 7 17:24:46 PST 2006
//
// Modifications:
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//   
// ****************************************************************************

class VIEWERCORE_API ViewStack
{
public:
    ViewStack();
    ViewStack(bool);
    ViewStack(const ViewStack &);
    ~ViewStack();

    void Clear();

    bool PopViewCurve(avtViewCurve &);
    bool PopView2D(avtView2D &);
    bool PopView3D(avtView3D &);
    bool PopViewAxisArray(avtViewAxisArray &);

    void PushViewCurve(const avtViewCurve &);
    void PushView2D(const avtView2D &);
    void PushView3D(const avtView3D &);
    void PushViewAxisArray(const avtViewAxisArray &);

    bool HasViewCurves() const;
    bool HasView2Ds() const;
    bool HasView3Ds() const;
    bool HasViewAxisArrays() const;

    // Assignment operator.
    void operator = (const ViewStack &);
private:
    bool         preventPopFirst;

    avtViewCurve     viewCurveStack[VSTACK_SIZE];
    int              viewCurveStackTop;
    avtView2D        view2DStack[VSTACK_SIZE];
    int              view2DStackTop;
    avtView3D        view3DStack[VSTACK_SIZE];
    int              view3DStackTop;
    avtViewAxisArray viewAxisArrayStack[VSTACK_SIZE];
    int              viewAxisArrayStackTop;
};

#endif
