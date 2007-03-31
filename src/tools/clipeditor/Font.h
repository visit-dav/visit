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
