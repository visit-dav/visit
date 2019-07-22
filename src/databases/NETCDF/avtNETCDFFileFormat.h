// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_NETCDF_FILE_FORMAT_H
#define AVT_NETCDF_FILE_FORMAT_H

class avtFileFormatInterface;

//
// Prototype for the file format interface factory function.
//
avtFileFormatInterface *
NETCDF_CreateFileFormatInterface(const char * const *, int, int);

#endif
