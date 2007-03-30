// ************************************************************************* //
//                                 Mesh.h                                    //
// ************************************************************************* //

#ifndef MESH_H
#define MESH_H
#include <siloobj_exports.h>

#include <iostream.h>
#include <silo.h>


//
// Enumerated Types
//

typedef enum
{
    CURVILINEAR                = 0,
    POINT,                    /* 1 */
    RECTILINEAR,              /* 2 */
    UNSTRUCTURED,             /* 3 */
    UNKNOWN                   /* 4 */
}  MESH_TYPE_e;

    
// ****************************************************************************
//  Class: Mesh
//
//  Purpose:
//      Keeps information about a mesh.
//
//  Data Members:
//      topology                The name of the topology object associated with
//                              the mesh.
//      intervalTree            The name of the interval tree associated with
//                              the coordinate field.
//      coordinateDimension     The dimension of the coordinates.
//      dataType                The data type of the coordinate field.
//      labels                  The labels associated with each coordinate 
//                              axis.
//      units                   The units associated with the coordinate field.
//      nDomains                The number of domains.
//      coordinates             The name of the value object with the mesh
//                              coordinates.
//      meshType                The type of the mesh.
//      
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
//  Modifications:
//      
//      Hank Childs, Sat Mar  4 09:53:12 PST 2000
//      Added meshType data member.
//
// ****************************************************************************

class SILOOBJ_API Mesh
{
  public:
                    Mesh();
    virtual        ~Mesh();

    char           *GetName() { return name; };
  
    void            PrintSelf(ostream &);
  
    void            Read(DBobject *);
    virtual void    Write(DBfile *);

    int             GetCoordinateDimension(void) {return coordinateDimension;};
    int             GetNDomains(void)            {return nDomains;};
    MESH_TYPE_e     GetMeshType(void)            {return meshType;};
    const char     *GetCoordinates(void)         {return coordinates;};

  protected:
    char           *topology;
    char           *intervalTree;
    int             coordinateDimension;
    int             dataType;
    char           *labels[3];
    char           *units[3];
    int             nDomains;
    char           *coordinates;
    MESH_TYPE_e     meshType;

    char           *name;

    // Constants
  public:
    static char * const   SILO_TYPE;
  protected:
    static int    const   SILO_NUM_COMPONENTS;

    static char * const   SILO_COORDINATE_DIMENSION_NAME;
    static char * const   SILO_COORDINATES_NAME;
    static char * const   SILO_DATA_TYPE_NAME;
    static char * const   SILO_INTERVAL_TREE_NAME;
    static char * const   SILO_LABEL_0_NAME;
    static char * const   SILO_LABEL_1_NAME;
    static char * const   SILO_LABEL_2_NAME;
    static char * const   SILO_MESH_TYPE_NAME;
    static char * const   SILO_N_DOMAINS_NAME;
    static char * const   SILO_TOPOLOGY_NAME;
    static char * const   SILO_UNIT_0_NAME;
    static char * const   SILO_UNIT_1_NAME;
    static char * const   SILO_UNIT_2_NAME;
};


#endif


