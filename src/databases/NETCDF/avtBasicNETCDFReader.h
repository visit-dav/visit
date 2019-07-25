// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_BASIC_NETCDF_READER_H
#define AVT_BASIC_NETCDF_READER_H
#include <avtNETCDFReaderBase.h>
#include <vectortypes.h>
#include <map>

class NETCDFFileObject;
class avtFileFormatInterface;

// ****************************************************************************
//  Class: avtBasicNETCDFReader
//
//  Purpose:
//      Reads in NETCDF files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Aug 10 15:21:14 PST 2005
//
//  Modifications:
//
// ****************************************************************************

class avtBasicNETCDFReader : public avtNETCDFReaderBase
{
public: 

                   avtBasicNETCDFReader(const char *filename, 
                                        NETCDFFileObject *);
                   avtBasicNETCDFReader(const char *filename);
                  ~avtBasicNETCDFReader();

    vtkDataSet    *GetMesh(int timeState, const char *);
    vtkDataArray  *GetVar(int timeState, const char *);
    void           PopulateDatabaseMetaData(int timeState, avtDatabaseMetaData *);

    void           CreateGlobalAttributesString(int nGlobalAtts, std::string &gaString);
protected:
    bool ReturnSpatialDimensionIndices(const intVector &dims, int sDims[3], int &nSDims) const;
    void ReturnDimStartsAndCounts(int timeState, const intVector &dims, 
                                  intVector &dimStarts, intVector &dimCounts) const;
private:
    // DATA MEMBERS
    bool                   meshNamesCreated;
    StringIntVectorMap     meshNameToDimensionsSizes;
    StringIntVectorMap     meshNameToNCDimensions;
    StringIntVectorMap     varToDimensionsSizes;

    int                    procNum;
    int                    procCount;
};

#endif
