// ************************************************************************* //
//                              OnionPeel.h                                  //
// ************************************************************************* //

#ifndef ONION_PEEL_H
#define ONION_PEEL_H
#include <siloobj_exports.h>

#include <visitstream.h>
#include <silo.h>


// ****************************************************************************
//  Class: OnionPeel
//
//  Purpose:
//      Takes in an ucd mesh and calculates the onion peel information.
//
//  Data Members:
//      domain      -  The domain to which the connectivity information 
//                     corresponds.
//      meshName    -  The name of the mesh.
//      list        -  The list of which zones correspond to which nodes.
//                     This is an array of size [nnodes]x[zones for node i],
//                     but is implemented as a one-dimensional array.
//      offset      -  The offsets into list that allow the two-dimensional
//                     array (list) to be implemented as a one-dimensional 
//                     array.  This is an array of size nnodes.
//      nnodes      -  The number of nodes for this mesh in this domain.
//
//  Programmer: Hank Childs
//  Creation:   January 5, 2000
//
// ****************************************************************************

class SILOOBJ_API OnionPeel
{
  friend         class BoundaryList;

  public:
                 OnionPeel();
                ~OnionPeel();

    void         AddMesh(DBucdmesh *, int);
    void         Write(DBfile *);

  private:
    int          domain;
    int         *list;
    char        *meshName;
    int          nnodes;
    int         *offset;

    // Class-scoped constants
    static char * const    SILO_BLOCK_NAME;
    static char * const    SILO_IDENTIFIER;
    static char * const    SILO_LIST_NAME;
    static char * const    SILO_OFFSET_NAME;
};


#endif


