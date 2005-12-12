#ifndef AVT_NETCDF_FILE_FORMAT_H
#define AVT_NETCDF_FILE_FORMAT_H

class avtFileFormatInterface;

//
// Prototype for the file format interface factory function.
//
avtFileFormatInterface *
NETCDF_CreateFileFormatInterface(const char * const *, int, int);

#endif
