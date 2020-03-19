// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#import <Cocoa/Cocoa.h>

#import "VisWinRenderingCocoaHideWindow.h"

void 
VisWinRenderingCocoa::HideRenderWindow(void* w)
{
    NSWindow* window = (NSWindow*) w;
    /// hide the window
    [window orderOut:0];
}
