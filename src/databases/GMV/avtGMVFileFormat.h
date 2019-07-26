// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtGMVFileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_GMV_FILE_FORMAT_H
#define AVT_GMV_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <map>

#include <avtDatabaseMetaData.h>

class avtPolyhedralSplit;

// ****************************************************************************
//  Class: avtGMVFileFormat
//
//  Purpose:
//      Reads in GMV files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 22 14:42:45 PST 2010
//
// ****************************************************************************

class avtGMVFileFormat : public avtSTMDFileFormat
{
public:
                       avtGMVFileFormat(const char *);
    virtual           ~avtGMVFileFormat();

    virtual int         GetCycle(void);
    virtual double      GetTime(void);

    virtual const char    *GetType(void)   { return "GMV"; };
    virtual void           FreeUpResources(void);

    virtual void           ActivateTimestep(void);

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

    virtual void          *GetAuxiliaryData(const char *var, int domain,
                                            const char *type, void *args,
                                            DestructorFunction &);
protected:
    void ReadData();
    std::string GetMeshName(const std::string &initial) const;

    struct MeshData
    {
        vtkDataSet         *dataset;
        vtkDataArray       *material;
        int                 materialCentering;
        avtPolyhedralSplit *polyhedralSplit;
    };

    // DATA MEMBERS
    bool                            dataRead;
    bool                            fileOpen;
    std::map<std::string, MeshData> meshes;
    avtDatabaseMetaData             md;
    int                             probCycle;
    double                          probTime;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
