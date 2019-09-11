// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtTSurfFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_TSURF_FILE_FORMAT_H
#define AVT_TSURF_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>


// ****************************************************************************
//  Class: avtTSurfFileFormat
//
//  Purpose:
//      Reads in TSurf files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Apr 11 10:27:39 PDT 2006
//
//  Modifications:
//
// ****************************************************************************

class avtTSurfFileFormat : public avtSTSDFileFormat
{
public:
                       avtTSurfFileFormat(const char *filename);
    virtual           ~avtTSurfFileFormat();

    virtual const char    *GetType(void)   { return "TSurf"; };
    virtual void           FreeUpResources(void); 

    virtual void           ActivateTimestep(void);

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

protected:
    struct FrameOfReference
    {
        float gridLon;
        float gridLat;
        float gridAZ;
        float zScale;
    };

    bool ReadFile(const char *, int nLines);
    bool ReadConfigFile();
    void ComputeCartesianCoord(float, float, float &, float &);

    // DATA MEMBERS
    bool                    frameOfReferenceSet;
    FrameOfReference        ref;
    vtkDataSet             *meshDS;
    std::string             title;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
