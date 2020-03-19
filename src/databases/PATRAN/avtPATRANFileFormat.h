// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtPATRANFileFormat.h                          //
// ************************************************************************* //

#ifndef AVT_PATRAN_FILE_FORMAT_H
#define AVT_PATRAN_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <vectortypes.h>

// ****************************************************************************
//  Class: avtPATRANFileFormat
//
//  Purpose:
//      Reads in PATRAN files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 28 13:51:07 PST 2005
//
//  Modifications:
//    Brad Whitlock, Mon Jun  8 10:12:39 PDT 2009
//    I added properties array to hold cell data.
//
// ****************************************************************************

class avtPATRANFileFormat : public avtSTSDFileFormat
{
public:
                       avtPATRANFileFormat(const char *filename);
    virtual           ~avtPATRANFileFormat();

    virtual void      *GetAuxiliaryData(const char *var, const char *type,
                                        void *args, DestructorFunction &);

    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    // virtual int       GetCycle(void);
    // virtual double    GetTime(void);
    //

    virtual const char    *GetType(void)   { return "PATRAN Neutral"; };
    virtual void           FreeUpResources(void); 

    virtual void           ActivateTimestep(void);

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

protected:
    bool ReadFile(const char *, int nLines);

    // DATA MEMBERS
    vtkDataSet            *meshDS;
    vtkDataArray          *elementMats;
    vtkDataArray          *elementIds;
    vtkDataArray          *properties[5];
    std::string            title;

    int                   *componentList;
    int                    componentListSize;
    stringVector           componentNames;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
