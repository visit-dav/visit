// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkCSGGrid - A fixed length bit field for use by the CSG classes.
// .SECTION Description
// This class provides a pre-defined fixed length bit field of a constant
// size to be used by all the CSG related classes

#ifndef __vtkCSGFixedLengthBitField_h
#define __vtkCSGFixedLengthBitField_h

#include <FixedLengthBitField.h>

// You must change both constants below when changing the length of the
// bit field. The length in the typedef is in bytes, so the number of bits
// is 8 times larger than the size. One of the bits is reserved for a validity
// flag, so that leaves one less than 8 times the number of bytes to be
// used for boundaries. This make VTK_CSG_MAX_BITS = 64 * 8 - 1.

#define VTK_CSG_MAX_BITS 511

typedef FixedLengthBitField<64> vtkCSGFixedLengthBitField;

#endif
