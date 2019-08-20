// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtADIOSFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_ADIOSSchema_FILE_FORMAT_H
#define AVT_ADIOSSchema_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>
#include <vector>
#include <string>
#include <map>

class avtFileFormatInterface;
class vtkRectilinearGrid;

// ****************************************************************************
//  Class: avtADIOSSchemaFileFormat
//
//  Purpose:
//      Reads in ADIOSSchema-ADIOS files as a plugin to VisIt.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
// ****************************************************************************

class avtADIOSSchemaFileFormat : public avtMTMDFileFormat
{
  public:
    static bool        Identify(const char *fname);
    static avtFileFormatInterface *CreateInterface(const char *const *list,
                                                   int nList,
                                                   int nBlock);
    static int NumberOfVertices(ADIOS_CELL_TYPE &ct);
    static int GetCellType(ADIOS_CELL_TYPE &ct);
    
    avtADIOSSchemaFileFormat(const char *);
    virtual  ~avtADIOSSchemaFileFormat();

    virtual void        GetCycles(std::vector<int> &);
    virtual void        GetTimes(std::vector<double> &);
    virtual int         GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "ADIOSSchema"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

  protected:
    bool             initialized;
    void             Initialize();

    virtual void     PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    ADIOSFileObject *fileObj;
    int numTimes;
    std::map<std::string, ADIOS_MESH *> meshes;
    std::map<std::string, ADIOS_VARINFO *> vars;
    std::map<std::string, std::string> varMeshes;

    vtkDataSet * MakeUniformMesh(MESH_UNIFORM *m, int ts, int dom);
    vtkDataSet * MakeRectilinearMesh(MESH_RECTILINEAR *m, int ts, int dom);
    vtkDataSet * MakeStructuredMesh(MESH_STRUCTURED *m, int ts, int dom);
    vtkDataSet * MakeUnstructuredMesh(MESH_UNSTRUCTURED *m, int ts, int dom);
};
#endif
