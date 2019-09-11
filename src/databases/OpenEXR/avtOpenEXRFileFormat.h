// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtOpenEXRFileFormat.h                         //
// ************************************************************************* //

#ifndef AVT_OpenEXR_FILE_FORMAT_H
#define AVT_OpenEXR_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>

// ****************************************************************************
//  Class: avtOpenEXRFileFormat
//
//  Purpose:
//      Reads in OpenEXR files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 27 17:24:29 PST 2017
//
// ****************************************************************************

class avtOpenEXRFileFormat : public avtSTSDFileFormat
{
public:
                       avtOpenEXRFileFormat(const char *filename);
    virtual           ~avtOpenEXRFileFormat();

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

    virtual const char    *GetType(void)   { return "OpenEXR"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

protected:
    class Internal;
    Internal *d;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
