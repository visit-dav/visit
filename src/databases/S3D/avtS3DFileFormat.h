// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtS3DFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_S3D_FILE_FORMAT_H
#define AVT_S3D_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vector>


// ****************************************************************************
//  Class: avtS3DFileFormat
//
//  Purpose:
//      Reads in S3D files as a plugin to VisIt.
//
//  Programmer: ahern -- generated by xml2avt
//  Creation:   Thu Jan 11 10:02:22 PDT 2007
//
// ****************************************************************************

class avtS3DFileFormat : public avtMTMDFileFormat
{
  public:
                       avtS3DFileFormat(const char *);
    virtual           ~avtS3DFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, const char *type,
    //                                     int timestep, int domain,void *args, 
    //                                     DestructorFunction &);
    //

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    // virtual void        GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "S3D"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

  protected:
    void                   CalculateSubpiece(int domain);

    bool opened;
    const char* mainFilename;
    int globalDims[3];
    int localDims[3];
    int offsetBy[3];
    int pos[3];
    int procs[3];
    int fileNCycles;
    std::vector<double> fileTimes;
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
    void                   OpenLogFile(void);
};


#endif
