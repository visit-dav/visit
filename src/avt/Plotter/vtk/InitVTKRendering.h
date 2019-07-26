// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               InitVTKRendering.h                          //
// ************************************************************************* //

#ifndef INIT_VTK_RENDERING_H
#define INIT_VTK_RENDERING_H
#include <plotter_exports.h>

// ****************************************************************************
//  Module: InitVTK
//
//  Purpose:
//      A central point for VTK initialization.
//
//  Programmer: Hank Childs
//  Creation:   April 24, 2001
//
//  Modifications:
//    Eric Brugger, Fri May 10 14:38:10 PDT 2013
//    I removed support for mangled mesa.
//
// ****************************************************************************

namespace InitVTKRendering
{
    PLOTTER_API void Initialize();
}

#endif
