// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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


class vtkCellData;
class vtkDataSetAttributes;
class vtkPointData;
class vtkPolyData;
class vtkRectilinearGrid;
class vtkStructuredGrid;
class vtkUnstructuredGrid;

class avtMeshMetaData;

class DBOptionsAttributes;

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
//    Hank Childs, Tue Sep 27 10:21:36 PDT 2005
//    Use virtual inheritance.
//
//    Jeremy Meredith, Tue Mar 27 11:39:24 EDT 2007
//    Added numblocks to the OpenFile method, and save off the actual
//    encountered mesh types, because we cannot trust the metadata.
//
//    Cyrus Harrison, Thu Aug 16 20:50:24 PDT 2007
//    Added dir to hold output directory.
//
//    Cyrus Harrison, Wed Feb 27 07:57:35 PST 2008
//    Added VTKZoneTypeToSiloZoneType helper method to help with 
//    DBPutZoneList2 migration. 
//
//    Mark C. Miller, Wed Jul 23 17:49:39 PDT 2008
//    Added bool to WriteUcdvars to handle point vars    
//
//    Mark C. Miller, Thu Jul 31 18:06:08 PDT 2008
//    Added option to write all data to a single file 
//
//    Brad Whitlock, Fri Mar 6 15:30:16 PST 2009
//    I added helper functions so we can share code when writing variables.
//    I also added code to export expressions.
//
//    Brad Whitlock, Tue May 19 12:22:38 PDT 2009
//    I added an argument to WriteUcdvars.
//
//    Eric Brugger, Mon Jun 22 16:35:47 PDT 2009
//    I modified the writer to handle the case where the meshes in a
//    multimesh or multivar were not all of the same type.
//
//    Mark C. Miller, Tue Jun 14 10:35:34 PDT 2016
//    Added arg to ConstrutMulti... methods. Added SingleFile() method to
//    subsume functionality of singleFile write option.
//
//    Alister Maguire, Mon Mar 18 14:08:39 PDT 2019
//    Added curCycle, curFTime, curDTime to avoid issues with static
//    members. 
//
// ****************************************************************************

class
avtSiloWriter : public virtual avtDatabaseWriter
{
  public:
                   avtSiloWriter(DBOptionsAttributes *);
    virtual       ~avtSiloWriter();

  protected:
    std::string         stem;
    std::string         dir;
    std::string         meshname;
    std::string         matname;
    int            nblocks;
    int            driver;
    int            nmeshes;
    int           *meshtypes;
    int           *chunkToFileMap;
    DBoptlist     *optlist;
    bool           singleFile;
    bool           checkSums;
    std::string    compressionParams;

    int            curCycle;
    float          curFTime;
    double         curDTime;

    // places to hold args passed in WriteHeaders
    const avtDatabaseMetaData           *headerDbMd;
    std::vector<std::string>             headerScalars;
    std::vector<std::string>             headerVectors;
    std::vector<std::string>             headerMaterials;

    // storage for per-block info to be written in DBPutMulti... calls 
    std::vector<double>               spatialMins;
    std::vector<double>               spatialMaxs;
    std::vector<int>                  zoneCounts;
    std::map<std::string,std::vector<double> > dataMins;
    std::map<std::string,std::vector<double> > dataMaxs;

    bool           WriteContextIsDefault();
    int            FileID();
    bool           ShouldCreateFile();

    virtual bool   CanHandleMaterials(void) { return true; };


    virtual void   OpenFile(const std::string &, int);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                const std::vector<std::string> &, 
                                const std::vector<std::string> &,
                                const std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);
    virtual void   WriteRootFile();

    void           ConstructMultimesh(DBfile *dbfile, const avtMeshMetaData *,
                       int const *, int const *);
    void           ConstructMultivar(DBfile *dbfile, const std::string &,
                       const avtMeshMetaData *, int const *, int const *);
    void           ConstructMultimat(DBfile *dbfile, const std::string &,
                       const avtMeshMetaData *, int const *);
    void           ConstructChunkOptlist(const avtDatabaseMetaData *);

    void           WriteCurvilinearMesh(DBfile *, vtkStructuredGrid *, int);
    void           WritePolygonalMesh(DBfile *, vtkPolyData *, int);
    void           WriteRectilinearMesh(DBfile *, vtkRectilinearGrid *, int);
    void           WriteUnstructuredMesh(DBfile *, vtkUnstructuredGrid *, int);

    void           WriteUcdvars(DBfile *, vtkPointData *, vtkCellData *, bool,
                                const unsigned char *);
    void           WriteQuadvars(DBfile *, vtkPointData *, vtkCellData *,
                                    int, int *);
    void           WriteMaterials(DBfile *, vtkCellData *, int);
    int            VTKZoneTypeToSiloZoneType(int);
    void           WriteUcdvarsHelper(DBfile *dbfile, vtkDataSetAttributes *ds, 
                                      bool isPointMesh, int centering,
                                      const unsigned char *gz);
    void           WriteQuadvarsHelper(DBfile *dbfile, vtkDataSetAttributes *ds,
                                       int ndims, int *dims, int centering);

    void           WriteExpressions(DBfile *dbfile);
};


#endif


