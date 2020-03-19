// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ----------------------------------------------------------------------------
// File:  Font.h
//
// Programmer: Jeremy Meredith
// Date:       August 11, 2003
// ----------------------------------------------------------------------------

#ifndef FONT_H
#define FONT_H

#include "Vector.h"

void
DrawCharacters(int nstr, char *str[],
               float scale, Vector origin, Vector up, Vector right);

void
DrawCharacter(char letter,
              float scale, Vector origin, Vector up, Vector right);
#endif
