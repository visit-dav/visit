// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISWINRENDERINGCOCOAHIDEWINDOW_H
#define VISWINRENDERINGCOCOAHIDEWINDOW_H

// ****************************************************************************
//  Class:  VisWinRenderingCocoa
//
//  Purpose:
//    Hides the render window when the engines render window is not OSMesa
//
//  Programmer:  Hari Krishnan
//  Creation:    December 13, 2007
//
//  Modifications:
//
// ****************************************************************************
class VisWinRenderingCocoa
{
public:
    static void HideRenderWindow(void* window);
};

#endif
