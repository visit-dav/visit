// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               InitVTK.h                                   //
// ************************************************************************* //

#ifndef INIT_VTK_H
#define INIT_VTK_H
#include <visit_vtk_exports.h>
#include <string>

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
//    Kathleen Biagas, Wed Oct 1, 2025
//    Add optional string argument. It will be passed to
//    InitVTKLite::Initialize, and used to create a callback to write their
//    log info to VisIt's debug log.
//
// ****************************************************************************

namespace InitVTK
{
    VISIT_VTK_API void Initialize(const std::string & = "");
}

#endif
