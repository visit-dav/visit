// ************************************************************************* //
//                             avtSiloWriter.h                               //
// ************************************************************************* //

#ifndef AVT_SILO_WRITER_H
#define AVT_SILO_WRITER_H

#include <avtDatabaseWriter.h>

#include <string>

#include <silo.h>


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
// ****************************************************************************

class
avtSiloWriter : public avtDatabaseWriter
{
  public:
                   avtSiloWriter();
    virtual       ~avtSiloWriter();

  protected:
    std::string    stem;
    std::string    meshname;
    std::string    matname;
    DBoptlist     *optlist;
 
    virtual bool   CanHandleMaterials(void) { return true; };

    virtual void   OpenFile(const std::string &);
    virtual void   WriteHeaders(const avtDatabaseMetaData *,
                                std::vector<std::string> &, 
                                std::vector<std::string> &,
                                std::vector<std::string> &);
    virtual void   WriteChunk(vtkDataSet *, int);
    virtual void   CloseFile(void);

    void           ConstructMultimesh(DBfile *dbfile, const avtMeshMetaData *);
    void           ConstructMultivar(DBfile *dbfile, const std::string &,
                                     const avtMeshMetaData *);
    void           ConstructMultimat(DBfile *dbfile, const std::string &,
                                     const avtMeshMetaData *);
    void           ConstructOptlist(const avtDatabaseMetaData *);

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


