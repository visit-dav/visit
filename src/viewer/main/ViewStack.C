/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <ViewStack.h>

// ****************************************************************************
// Method: ViewStack constructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 8 17:01:56 PST 2006
//
// Modifications:
//   
// ****************************************************************************

ViewStack::ViewStack()
{
    preventPopFirst = false;
    viewCurveStackTop = -1;
    view2DStackTop = -1;
    view3DStackTop = -1;
}

ViewStack::ViewStack(bool val)
{
    preventPopFirst = val;
    viewCurveStackTop = -1;
    view2DStackTop = -1;
    view3DStackTop = -1;
}

ViewStack::ViewStack(const ViewStack &obj)
{
    int i;
    viewCurveStackTop = obj.viewCurveStackTop;
    for(i = 0; i < viewCurveStackTop+1; ++i)
        viewCurveStack[i] = obj.viewCurveStack[i];

    view2DStackTop = obj.view2DStackTop;
    for(i = 0; i < view2DStackTop+1; ++i)
        view2DStack[i] = obj.view2DStack[i];

    view3DStackTop = obj.view3DStackTop;
    for(i = 0; i < view3DStackTop+1; ++i)
        view3DStack[i] = obj.view3DStack[i];

    preventPopFirst = obj.preventPopFirst;
}

// ****************************************************************************
// Method: ViewStack destructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 8 17:02:10 PST 2006
//
// Modifications:
//   
// ****************************************************************************

ViewStack::~ViewStack()
{
}

// ****************************************************************************
// Method: ViewStack::operator =
//
// Purpose: 
//   Assignment operator
//
// Arguments:
//   obj : The object being assigned into this.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 8 17:02:21 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
ViewStack::operator = (const ViewStack &obj)
{
    int i;
    viewCurveStackTop = obj.viewCurveStackTop;
    for(i = 0; i < viewCurveStackTop+1; ++i)
        viewCurveStack[i] = obj.viewCurveStack[i];

    view2DStackTop = obj.view2DStackTop;
    for(i = 0; i < view2DStackTop+1; ++i)
        view2DStack[i] = obj.view2DStack[i];

    view3DStackTop = obj.view3DStackTop;
    for(i = 0; i < view3DStackTop+1; ++i)
        view3DStack[i] = obj.view3DStack[i];

    preventPopFirst = obj.preventPopFirst;
}

// ****************************************************************************
// Method: ViewStack::Clear
//
// Purpose: 
//   Clear the view stacks.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 8 17:02:50 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
ViewStack::Clear()
{
    viewCurveStackTop = -1;
    view2DStackTop = -1;
    view3DStackTop = -1;
}

// ****************************************************************************
// Method: ViewStack::PopView*
//
// Purpose: 
//   Pops a view object and returns it in the referenced object.
//
// Arguments:
//   v : The view object.
//
// Returns:    True if an object was returned; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 8 17:03:06 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
ViewStack::PopViewCurve(avtViewCurve &v)
{
    bool retval = false;
    // Pop the top off of the stack
    if(preventPopFirst && viewCurveStackTop > 0)
    {
        --viewCurveStackTop;
        v = viewCurveStack[viewCurveStackTop];
        retval = true;
    }
    else if(viewCurveStackTop >= 0)
    {
        v = viewCurveStack[viewCurveStackTop];
        --viewCurveStackTop;
        retval = true;
    }

    return retval;
}

bool
ViewStack::PopView2D(avtView2D &v)
{
    bool retval = false;
    // Pop the top off of the stack
    if(preventPopFirst && view2DStackTop > 0)
    {
        --view2DStackTop;
        v = view2DStack[view2DStackTop];
        retval = true;
    }
    else if(view2DStackTop >= 0)
    {
        v = view2DStack[view2DStackTop];
        --view2DStackTop;
        retval = true;
    }

    return retval;
}

bool
ViewStack::PopView3D(avtView3D &v)
{
    bool retval = false;
    // Pop the top off of the stack
    if(preventPopFirst && view3DStackTop > 0)
    {
        --view3DStackTop;
        v = view3DStack[view3DStackTop];
        retval = true;
    }
    else if(view3DStackTop >= 0)
    {
        v = view3DStack[view3DStackTop];
        --view3DStackTop;
        retval = true;
    }
    return retval;
}

// ****************************************************************************
// Method: ViewStack::PushView*
//
// Purpose: 
//   Pushes a view onto the view stack.
//
// Arguments:
//   v : The view to push.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 8 17:03:49 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
ViewStack::PushViewCurve(const avtViewCurve &v)
{
    if(viewCurveStackTop == VSTACK_SIZE - 1)
    {
        // Shift down
        for(int i = 0; i < VSTACK_SIZE - 1; ++i)
            viewCurveStack[i] = viewCurveStack[i+1];
    }
    else
        ++viewCurveStackTop;

    viewCurveStack[viewCurveStackTop] = v;
}

void
ViewStack::PushView2D(const avtView2D &v)
{
    if(view2DStackTop == VSTACK_SIZE - 1)
    {
        // Shift down
        for(int i = 0; i < VSTACK_SIZE - 1; ++i)
            view2DStack[i] = view2DStack[i+1];
    }
    else
        ++view2DStackTop;

    view2DStack[view2DStackTop] = v;
}

void
ViewStack::PushView3D(const avtView3D &v)
{
    if(view3DStackTop == VSTACK_SIZE - 1)
    {
        // Shift down
        for(int i = 0; i < VSTACK_SIZE - 1; ++i)
            view3DStack[i] = view3DStack[i+1];
    }
    else
        ++view3DStackTop;

    view3DStack[view3DStackTop] = v;
}

// ****************************************************************************
// Method: ViewStack::HasView*
//
// Purpose: 
//   Returns whether the view stack has an object of the specified type.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 8 17:04:18 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
ViewStack::HasViewCurves() const
{
    int empty = preventPopFirst ? 0 : -1;
    return viewCurveStackTop != empty;
}

bool
ViewStack::HasView2Ds() const
{
    int empty = preventPopFirst ? 0 : -1;
    return view2DStackTop != empty;
}

bool
ViewStack::HasView3Ds() const
{
    int empty = preventPopFirst ? 0 : -1;
    return view3DStackTop != empty;
}

