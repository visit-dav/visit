// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtVis5DFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Vis5D_FILE_FORMAT_H
#define AVT_Vis5D_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>

#include <vector>

#include <v5d.h>

// ****************************************************************************
//  Class: avtVis5DFileFormat
//
//  Purpose:
//      Reads in Vis5D files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jul 14 14:07:16 PST 2006
//
//  Modifications:
//
// ****************************************************************************

class avtVis5DFileFormat : public avtMTSDFileFormat
{
public:
                       avtVis5DFileFormat(const char *);
    virtual           ~avtVis5DFileFormat();

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    virtual void           GetCycles(std::vector<int> &);
    //virtual void           GetTimes(std::vector<double> &);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "Vis5D"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

protected:
    // DATA MEMBERS
    v5dstruct             *v5dfile;

    void                   Initialize();
    vtkDataSet            *CreateCurvilinearMesh();

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
};


#endif
