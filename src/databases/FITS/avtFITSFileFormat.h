// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.


// ************************************************************************* //
//                            avtFITSFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_FITS_FILE_FORMAT_H
#define AVT_FITS_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <vectortypes.h>
#include <map>

#include <fitsio.h>

// ****************************************************************************
//  Class: avtFITSFileFormat
//
//  Purpose:
//      Reads in FITS files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 19 09:54:58 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

class avtFITSFileFormat : public avtSTSDFileFormat
{
public:
                       avtFITSFileFormat(const char *filename);
    virtual           ~avtFITSFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, const char *type,
    //                                  void *args, DestructorFunction &);
    //

    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    // virtual int       GetCycle(void);
    // virtual double    GetTime(void);
    //

    virtual const char    *GetType(void)   { return "FITS"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

protected:
    // DATA MEMBERS
    typedef std::map<std::string, intVector>   MeshNameMap;
    typedef std::map<std::string, int>         StringIntMap;
    typedef std::map<std::string, std::string> StringStringMap;

    fitsfile              *fits;
    MeshNameMap            meshDimensions;
    StringIntMap           varToHDU;
    StringStringMap        varToMesh;

    bool                   GetKeywordValue(const char *, char *);
    void                   Initialize(avtDatabaseMetaData *);
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
