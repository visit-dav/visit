// ************************************************************************* //
//                          StateTableOfContents.h                           //
// ************************************************************************* //

#ifndef STATE_TABLE_OF_CONTENTS_H
#define STATE_TABLE_OF_CONTENTS_H
#include <siloobj_vtk_exports.h>

#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkScalars.h>
#include <vtkStructuredGrid.h>

#include <Material.h>
#include <Mesh_VTK.h>
#include <TableOfContents.h>


// ****************************************************************************
//  Class: StateTableOfContents
//
//  Purpose:
//      Reads in the meshtvprep object based SILO format for a state.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
//  Modifications:
//  
//    Hank Childs, Sat Mar  4 11:00:47 PST 2000
//    Added a GetCoords routine for rectilinear meshes.
//
//    Hank Childs, Mon Apr  3 15:21:51 PDT 2000
//    Added a GetCoords routine for curvilinear meshes.
//
//    Hank Childs, Wed Apr 12 21:30:02 PDT 2000
//    Added a GetMetaData method.  Removed methods GetVarRange,
//    GetMeshDomainList, and GetVarDomainList.
//
//    Hank Childs, Thu Aug 10 11:12:08 PDT 2000
//    Added GetNDomains for meshes.
//
//    Hank Childs, Fri Sep 15 18:10:47 PDT 2000
//    Get centering type of a variable.
//
// ****************************************************************************

class SILOOBJ_VTK_API StateTableOfContents : public TableOfContents
{
    friend class                avtSiloObjDatabase;

  public:
                                StateTableOfContents();
    virtual                    ~StateTableOfContents();

    const char                 *GetMeshName(const char *);
    MESH_TYPE_e                 GetMeshType(const char *);
    void                        GetCoords(const char *, const int *, int, 
                                          vtkUnstructuredGrid **);
    void                        GetCoords(const char *, const int *, int, 
                                          vtkRectilinearGrid **);
    void                        GetCoords(const char *, const int *, int, 
                                          vtkStructuredGrid **);
    void                        GetVar(const char *, const int *, int, 
                                       vtkScalars **);
    const IntervalTree_VTK     *GetMetaData(const char *);

    int                         GetNDomains(const char *);
    int                         GetCentering(const char *);

    virtual Field_VTK          *GetField(const char *);
    virtual IntervalTree_VTK   *GetIntervalTree(const char *);
    virtual Value_VTK          *GetValue(const char *);

    virtual void                UpdateReferences(void);

  protected:
    Field_VTK                 **fields;
    int                         fieldsN;
    IntervalTree_VTK          **trees;
    int                         treesN;
    Material                   *mats;
    int                         matsN;
    Mesh_VTK                   *meshes;
    int                         meshesN;
    Value_VTK                 **values;
    int                         valuesN;

    // Protected Methods
    virtual void                ReadFile(const char *);
    Field_VTK                  *ResolveField(const char *);
    Mesh_VTK                   *ResolveMesh(const char *);
};


#endif


