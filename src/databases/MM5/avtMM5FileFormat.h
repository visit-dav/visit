// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtMM5FileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_MM5_FILE_FORMAT_H
#define AVT_MM5_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>
#include <MM5.h>

#include <map>
#include <vectortypes.h>

// ****************************************************************************
//  Class: avtMM5FileFormat
//
//  Purpose:
//      Reads in MM5 files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 11 16:39:09 PST 2006
//
// ****************************************************************************

class avtMM5FileFormat : public avtMTSDFileFormat
{
public:
                       avtMM5FileFormat(const char *);
    virtual           ~avtMM5FileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, const char *type,
    //                                     int timestep, void *args, 
    //                                     DestructorFunction &);
    //

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    // virtual void        GetCycles(std::vector<int> &);

    virtual void           GetTimes(std::vector<double> &);
    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "MM5"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

protected:
    typedef std::map<std::string, intVector> MeshNameMap;

    // DATA MEMBERS
    mm5_file_t            *mm5file;
    bool                   meshNamesCreated;
    MeshNameMap            meshNames;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
    void                   Initialize();
};


#endif
