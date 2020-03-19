// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtDyna3DFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_DYNA3D_FILE_FORMAT_H
#define AVT_DYNA3D_FILE_FORMAT_H
#include <visitstream.h>
#include <avtSTSDFileFormat.h>
#include <string>
#include <vector>

#include <Dyna3DFile.h>

// ****************************************************************************
//  Class: avtDyna3DFileFormat
//
//  Purpose:
//      Reads in Dyna3D files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 27 14:00:02 PST 2006
//
//  Modifications:
//    Brad Whitlock, Fri Aug  1 11:41:18 PDT 2008
//    Added support for material strength.
//
//    Brad Whitlock, Wed Mar 11 16:18:45 PDT 2009
//    I changed how materials are read.
//
//    Brad Whitlock, Fri Dec 16 11:18:36 PST 2011
//    Use the Dyna3DFile class to make it easier to sync up with changes in
//    DGEM/SDM.
//
// ****************************************************************************

class avtDyna3DFileFormat : public avtSTSDFileFormat
{
public:
                       avtDyna3DFileFormat(const char *filename);
    virtual           ~avtDyna3DFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    virtual void      *GetAuxiliaryData(const char *var, const char *type,
                                        void *args, DestructorFunction &);
    
    virtual const char    *GetType(void)   { return "Dyna3D input data"; };
    virtual void           FreeUpResources(void); 

    virtual void           ActivateTimestep(void);

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

protected:
    // DATA MEMBERS
    Dyna3DFile file;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
