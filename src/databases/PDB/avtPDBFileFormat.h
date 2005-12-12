#ifndef AVT_PDB_FILE_FORMAT_H
#define AVT_PDB_FILE_FORMAT_H
class avtFileFormatInterface;

//
// Prototype for the file format interface factory function.
//
avtFileFormatInterface *
PDB_CreateFileFormatInterface(const char * const *, int, int);

#endif
