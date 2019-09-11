// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               arial_font.h                                //
// ************************************************************************* //

#ifndef ARIAL_FONT_H
#define ARIAL_FONT_H


class vtkPolyData;


//  Modifications:
//    Jeremy Meredith, Thu Aug  7 14:52:31 EDT 2008
//    Use const string inputs since we're not modifying them.
//
vtkPolyData *CreateText(int nstr, const char *const str[]);


#endif


