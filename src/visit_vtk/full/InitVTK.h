// ************************************************************************* //
//                               InitVTK.h                                   //
// ************************************************************************* //

#ifndef INIT_VTK_H
#define INIT_VTK_H
#include <visit_vtk_exports.h>

// ****************************************************************************
//  Module: InitVTK
//
//  Purpose:
//      A central point for VTK initialization.
//
//  Programmer: Hank Childs
//  Creation:   April 24, 2001
//
// ****************************************************************************

namespace InitVTK
{
    VISIT_VTK_API void Initialize();
    VISIT_VTK_API void ForceMesa();
    VISIT_VTK_API void UnforceMesa();
}

#endif
