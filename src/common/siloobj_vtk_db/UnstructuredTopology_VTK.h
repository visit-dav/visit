// ************************************************************************* //
//                       UnstructuredTopology_VTK.h                          //
// ************************************************************************* //

#ifndef UNSTRUCTURED_TOPOLOGY_VTK_H
#define UNSTRUCTURED_TOPOLOGY_VTK_H
#include <siloobj_vtk_exports.h>

#include <iostream.h>

#include <silo.h>

#include <vtkUnstructuredGrid.h>

#include <UnstructuredTopology.h>
#include <Value_VTK.h>


// 
// Forward declaration of classes.
//

class TableOfContents;


// ****************************************************************************
//  Class: UnstructuredTopology_VTK
//
//  Purpose:
//      A derived type of UnstructuredTopology that allows for accessing
//      its node lists.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2000
//
// ****************************************************************************

class SILOOBJ_VTK_API UnstructuredTopology_VTK : public UnstructuredTopology
{
  public:
                     UnstructuredTopology_VTK();
    virtual         ~UnstructuredTopology_VTK();
  
    void             GetZones(const int *, int, vtkUnstructuredGrid **);

    void             UpdateReferences(TableOfContents *);

  protected:
    Value_VTK       *aliasedNodeListValue;
    Value_VTK       *aliasedShapeCountValue;
    Value_VTK       *aliasedShapeTypeValue;
    Value_VTK       *aliasedShapeSizeValue;

    TableOfContents *toc;

    // Protected Methods
    void             CreateValues(Value_VTK *, Value_VTK *, Value_VTK *, 
                                  Value_VTK *);
};


#endif


