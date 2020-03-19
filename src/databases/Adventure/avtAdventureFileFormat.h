// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtAdventureFileFormat.h                       //
// ************************************************************************* //

#ifndef AVT_ADVENTURE_FILE_FORMAT_H
#define AVT_ADVENTURE_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <AdvData.h>
#include <MshData.h>

// ****************************************************************************
// Class: avtAdventureFileFormat
//
// Purpose:
//     Reads in Adventure files as a plugin to VisIt.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 14 15:59:24 PST 2009
//
// Modifications:
//
// ****************************************************************************

class avtAdventureFileFormat : public avtSTMDFileFormat
{
public:
                       avtAdventureFileFormat(const char *);
    virtual           ~avtAdventureFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, int domain,
    //                                     const char *type, void *args, 
    //                                     DestructorFunction &);
    //

    //
    // If you know the cycle number, overload this function.
    // Otherwise, VisIt will make up a reasonable one for you.
    //
    // virtual int         GetCycle(void);
    //

    virtual const char    *GetType(void)   { return "Adventure"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

protected:
    void                   Initialize();

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    std::string filename;
    bool        usingAdvData;
    bool        initialized;
    AdvData     advData;
    MshData     mshData;
};


#endif
