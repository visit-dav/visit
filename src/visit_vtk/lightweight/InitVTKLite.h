// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             InitVTKLite.h                                 //
// ************************************************************************* //

#ifndef INIT_VTK_LITE_H
#define INIT_VTK_LITE_H

#include <visit_vtk_light_exports.h>
#include <string>

// ****************************************************************************
//  Module: InitVTKLite
//
//  Purpose:
//      A central point for VTK initialization.
//
//  Programmer: Hank Childs
//  Creation:   January 22, 2004
//
//  Modifications:
//    Kathleen Biagas, Wed Oct 1, 2025
//    Add string argument. It will be used for creating a vtkLogger
//    callback to write their log info to VisIt's debug log.
//
// ****************************************************************************

namespace InitVTKLite
{
    VISIT_VTK_LIGHT_API void Initialize(const std::string &);
}

#endif


