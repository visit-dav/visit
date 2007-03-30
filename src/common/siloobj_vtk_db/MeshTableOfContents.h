// ************************************************************************* //
//                           MeshTableOfContents.h                           //
// ************************************************************************* //

#ifndef MESH_TABLE_OF_CONTENTS_H
#define MESH_TABLE_OF_CONTENTS_H
#include <siloobj_vtk_exports.h>

#include <vtkUnstructuredGrid.h>

#include <StructuredTopology.h>
#include <TableOfContents.h>
#include <UnstructuredTopology_VTK.h>


// ****************************************************************************
//  Class: MeshTableOfContents
//
//  Purpose:
//      Reads in the meshtvprep object based SILO format for a state.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

class SILOOBJ_VTK_API MeshTableOfContents : public TableOfContents
{
  public:
                                MeshTableOfContents();
    virtual                    ~MeshTableOfContents();

    void                        GetZones(const char *, const int *, int, 
                                         vtkUnstructuredGrid **);

    virtual Field_VTK          *GetField(const char *)        { return NULL; };
    virtual IntervalTree_VTK   *GetIntervalTree(const char *) { return NULL; };
    virtual Value_VTK          *GetValue(const char *);

    virtual void                UpdateReferences(void);

  protected:
    StructuredTopology         *structuredTopology;
    int                         structuredTopologyN;
    UnstructuredTopology_VTK   *unstructuredTopology;
    int                         unstructuredTopologyN;
    Value_VTK                 **values;
    int                         valuesN;

    // Protected Methods
    virtual void                ReadFile(const char *);
};


#endif


