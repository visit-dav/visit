// ************************************************************************* //
//                                Mesh_VTK.h                                 //
// ************************************************************************* //

#ifndef MESH_VTK_H
#define MESH_VTK_H
#include <siloobj_vtk_exports.h>

#include <iostream.h>

#include <silo.h>

#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <Field_VTK.h>
#include <IntervalTree_VTK.h>
#include <Mesh.h>


//
//  Forward declaration of classes.
//

class  TableOfContents;


// ****************************************************************************
//  Class: Mesh_VTK
//
//  Purpose:
//      A derived type of Mesh that constructs VTK objects.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2000
//
//  Modifications:
//    
//    Hank Childs, Sat Mar  4 10:36:28 PST 2000
//    Added GetCoords routine for rectilinear meshes.
//
//    Hank Childs, Mon Apr  3 15:06:55 PDT 2000
//    Added GetCoords routine for curvilinear meshes.
//
//    Hank Childs, Wed Apr 12 21:47:40 PDT 2000
//    Removed method GetDomainList, added method GetMetaData.
//
// ****************************************************************************

class SILOOBJ_VTK_API Mesh_VTK : public Mesh
{
  public:
                             Mesh_VTK();
    virtual                 ~Mesh_VTK();

    void                     GetCoords(const int *, int, 
                                       vtkUnstructuredGrid **);
    void                     GetCoords(const int *, int, 
                                       vtkRectilinearGrid **);
    void                     GetCoords(const int *, int, vtkStructuredGrid **);
    const IntervalTree_VTK  *GetMetaData(void);

    void                     UpdateReferences(TableOfContents *);

  protected:
    // The table of contents that holds this mesh.
    TableOfContents         *toc;

    // The coordinate field (as a pointed to the _VTK type).
    Field_VTK               *coordsField;
};


#endif


