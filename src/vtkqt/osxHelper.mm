// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <Cocoa/Cocoa.h>
#include "osxHelper.h"

// ****************************************************************************
//  Method: disableGLHiDPI
//
//  Purpose:
//    Disable using High DPI (retina display) which creates an OpenGL backing
//    surface with a resolution greater than 1 pixel per point. The caller will
//    need to ensure that Qt is using Cocoa since win_id can be a pointer to 
//    an HIViewRef (Carbon) or NSView (Cocoa).
//
//    Code courtesy of Simon Esneault simon.esneault@gmail.com
//    http://public.kitware.com/pipermail/vtkusers/2015-February/090117.html
//
//  Arguments:
//    win_id : window system identifier. The type depends on the framework
//             Qt was linked against. If Qt is using Carbon, win_id is 
//             actually an HIViewRef. If Qt is using Cocoa, win_id is a pointer
//             to an NSView.
//  
//  Programmer: Kevin Griffin
//  Creation:   Thu Apr 27 11:32:52 PDT 2017
//
//  Modifications:
//
// ****************************************************************************
void disableGLHiDPI(long win_id)
{
    NSView* view = reinterpret_cast<NSView*>(win_id);
    [view setWantsBestResolutionOpenGLSurface:NO];
}
