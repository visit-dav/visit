// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtADIOSFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_ADIOS_FILE_FORMAT_H
#define AVT_ADIOS_FILE_FORMAT_H

class avtFileFormatInterface;

//
// Prototype for the file format interface factory function.
//
avtFileFormatInterface *
ADIOS2_CreateFileFormatInterface(const char * const *, int, int);

#endif
