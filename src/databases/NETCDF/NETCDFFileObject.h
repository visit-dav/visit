// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef NETCDF_FILE_OBJECT_H
#define NETCDF_FILE_OBJECT_H
#include <string>
#include <visitstream.h>

typedef enum {NO_TYPE, CHARARRAY_TYPE, UCHARARRAY_TYPE, SHORTARRAY_TYPE,
              INTEGERARRAY_TYPE, FLOATARRAY_TYPE,
              DOUBLEARRAY_TYPE, LONGARRAY_TYPE} TypeEnum;

// ****************************************************************************
// Class: NETCDFFileObject
//
// Purpose:
//   Abstract the NETCDF file a little.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 12 10:07:08 PDT 2005
//
// Modifications:
//   Mark C. Miller, Tue Aug 15 15:28:11 PDT 2006
//   Added method for partial read to support on-the-fly domain decomp
//
//   Brad Whitlock, Tue Oct 27 14:28:08 PDT 2009
//   Added GetDimensionInfo method.
//
// ****************************************************************************

class NETCDFFileObject
{
public:
    NETCDFFileObject(const char *name); 
    virtual ~NETCDFFileObject();

    bool IsOpen() const;
    bool Open();
    void Close();
    const std::string &GetName() const;
    int  GetFileHandle();

    bool ReadAttribute(const char *attname, TypeEnum *type, int *ndims,
                       int **dims, void **value);
    bool ReadAttribute(const char *varname, const char *attname,
                       TypeEnum *type, int *ndims, int **dims, void **value);
    bool ReadAttributeAsDouble(const char *varname, const char *attname,
                               TypeEnum *origType, double **values, int *nvalues);
    // Convenience functions
    bool ReadStringAttribute(const char *varname, const char *attname,
                             std::string &attval);
    bool ReadStringAttribute(const char *attname, std::string &attval);

    bool InqVariable(const char *varname, TypeEnum *, int *ndims, int **dims);
    bool ReadVariable(const char *varname, TypeEnum *type, int *ndims,
                      int **dims, void **values);
    bool ReadVariableInto(const char *varname, TypeEnum t, void *arr);
    bool ReadVariableInto(const char *varname, TypeEnum t, 
             const int *const starts, const int *const counts, void *arr);
    bool ReadVariableIntoAsFloat(const char *varname, float *arr);

    bool GetVarId(const char *name, int *varid);

    bool GetDimensionInfo(const char *dName, size_t *size);

    void HandleError(int status) const;
    void PrintFileContents(ostream &os);
private:
    bool AutoOpen();

    std::string filename;
    int         ncid;
};

//
// Functions to free memory.
//

template <class T>
void free_mem(T *);

void free_void_mem(void *, TypeEnum);

#endif
