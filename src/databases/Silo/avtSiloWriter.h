// ************************************************************************* //
//                             avtSiloWriter.h                               //
// ************************************************************************* //

#ifndef AVT_SILO_WRITER_H
#define AVT_SILO_WRITER_H

#include <avtDatabaseWriter.h>

#include <map>
#include <string>
#include <vector>

#include <silo.h>

// Silo does not define its own compile-time symbols for API versions.
// Define SILO_VERSION_4_4 if you have Silo 4.4 or later and want to
// write extents on Silo's multi-objects.
#define SILO_VERSION_4_4

using std::map;
using std::string;
using std::vector;

class vtkCellData;
class vtkPointData;
class vtkPolyData;
class vtkRectilinearGrid;
class vtkStructuredGrid;
class vtkUnstructuredGrid;

struct avtMeshMetaData;


// ****************************************************************************
//  Class: avtSiloWriter
//
//  Purpose:
//      A module that writes out Silo files.
//
//  Programmer: Hank Childs
//  Creation:   September 11, 2003
//
//  Modifications
//    Mark C. Miller, Tue Mar  9 07:03:56 PST 2004
//    Added data members to hold args passed in WriteHeaders to facilitate
//    writing extents on multi-objects in CloseFile()
//
// ****************************************************************************

class
avtSiloWriter : public avtDatabaseWriter
{
  public:
                   avtSiloWriter();
    virtual       ~avtSiloWriter();

  protected:
    string         stem;
    string         meshname;
    string         matname;
    DBoptlist     *optlist;

    // places to hold args passed in WriteHeaders
    const avtDatabaseMetaData *headerDbMd;
    vector<string>             headerScalars;
    vector<string>             headerVectors;
    vector<string>             headerMaterials;

    // storage for per-block info to be written in DBPutMulti... calls 
    vector<double>               spatialMins;
    vector<double>               spatialMaxs;
    vector<int>                  zoneCounts;
    map<string,vector<double> > dataMins;
    map<string,vector<double> > dataMaxs;

    virtual bool   CanHandleMaterials(void) { return true; };

    virtual void   OpenFile(const string &);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                vector<string> &, 
                                vector<string> &,
                                vector<string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);

    void           ConstructMultimesh(DBfile *dbfile, const avtMeshMetaData *);
    void           ConstructMultivar(DBfile *dbfile, const string &,
                                     const avtMeshMetaData *);
    void           ConstructMultimat(DBfile *dbfile, const string &,
                                     const avtMeshMetaData *);
    void           ConstructChunkOptlist(const avtDatabaseMetaData *);

    void           WriteCurvilinearMesh(DBfile *, vtkStructuredGrid *, int);
    void           WritePolygonalMesh(DBfile *, vtkPolyData *, int);
    void           WriteRectilinearMesh(DBfile *, vtkRectilinearGrid *, int);
    void           WriteUnstructuredMesh(DBfile *, vtkUnstructuredGrid *, int);

    void           WriteUcdvars(DBfile *, vtkPointData *, vtkCellData *);
    void           WriteQuadvars(DBfile *, vtkPointData *, vtkCellData *,
                                    int, int *);
    void           WriteMaterials(DBfile *, vtkCellData *, int);
};


#endif


