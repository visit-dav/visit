// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_NETCDF_READER_BASE_H
#define AVT_NETCDF_READER_BASE_H
#include <map>
#include <string>
#include <vectortypes.h>

#include <NETCDFFileObject.h>

class avtScalarMetaData;
class avtDatabaseMetaData;
class vtkDataSet;
class vtkDataArray;

// ****************************************************************************
// Class: avtNETCDFReaderBase
//
// Purpose:
//   This class is a base class for "Reader" classes. which I mean to distinguish
//   as the class that does the actual NETCDF reading. The various FileFormat 
//   classes call a reader to do their work. There are MTSD and STSD versions
//   of the FileFormat which call the same reader class. This permits the plugin 
//   to serve up the best avtFileFormatInterface for the data with respect to 
//   whether it is "MT" or "ST". Thus, the FileFormat classes become a very
//   thin veneer on top of the Reader class. We do this mostly to work around a
//   file grouping issue in avtDatabase where it can't group MT files. The other
//   reason is that we don't want to have an ST or MT file format because we
//   don't know which it is until the file is opened.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 27 14:39:32 PDT 2009
//
// Modifications:
//   Eric Brugger, Mon Nov 16 07:59:52 PST 2009
//   I added ReadTimeAttribute and ReadCycleAttribute.
//
//   Brad Whitlock, Thu Jan  5 16:52:54 PST 2012
//   Add HandleMissingData.
//
// ****************************************************************************

class avtNETCDFReaderBase
{
public:
                   avtNETCDFReaderBase(const char *);
                   avtNETCDFReaderBase(const char *, NETCDFFileObject *);
    virtual       ~avtNETCDFReaderBase();

    void           GetCycles(std::vector<int> &);
    void           GetTimes(std::vector<double> &);

    int            GetNTimesteps();
    void           FreeUpResources(); 

    static bool GetTimeDimension(NETCDFFileObject *, int &ncdim, int &nts, std::string &name);
protected:
    float *ReadTimeAttribute();
    int    ReadCycleAttribute();
    float *ReadArray(const char *varname);
    bool   HandleMissingData(const std::string &varname, avtScalarMetaData *smd);
    bool   ReadScaleAndOffset(const std::string &var,
                              TypeEnum *t, double *scale, double *offset);
    vtkDataArray *ApplyScaleAndOffset(const std::string &realvar,
                                      vtkDataArray *arr);

    typedef std::map<std::string, intVector>   StringIntVectorMap;
    typedef std::map<std::string, std::string> StringStringMap;

    // DATA MEMBERS
    NETCDFFileObject      *fileObject;
};

#endif
