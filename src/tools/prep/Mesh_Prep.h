// ************************************************************************* //
//                               MeshPrep.h                                  //
// ************************************************************************* //

#ifndef MESH_PREP_H
#define MESH_PREP_H

#include <Mesh.h>


// ****************************************************************************
//  Class: Mesh_Prep
//
//  Purpose:
//      A derived class of Mesh that is meant specifically for the meshtvprep
//      project.
//
//  Data Members:
//      read...  -  A boolean value which keeps track of whether that field
//                  has been read in.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
//  Modifications:
//     
//      Hank Childs, Sat Mar  4 10:12:08 PST 2000
//      Changed signature of ReadMesh for meshType.  Added data member
//      readMeshType.
//
// ****************************************************************************

class Mesh_Prep : public Mesh
{
  public:
                    Mesh_Prep();
    virtual        ~Mesh_Prep();

    void            ReadMesh(DBpointmesh *);
    void            ReadMesh(DBquadmesh *);
    void            ReadMesh(DBucdmesh *);

    void            SetName(char *);
    void            SetDomains(int);
    void            SetId(int);
  
    void            Consolidate(void);
    void            WrapUp(void);
    virtual void    Write(DBfile *);

  private:
    bool            readCoordinateDimension;
    bool            readDataType;
    bool            readLabels;
    bool            readMeshType;
    bool            readUnits;

    // Private Methods
    void         ReadMesh(int, char **, char **, int, MESH_TYPE_e);

    // Constants
    static char * const   SILO_LOCATION;
    static char * const   COORDS_NAME;
};


#endif


