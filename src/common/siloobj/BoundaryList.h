// ************************************************************************* //
//                             BoundaryList.h                                //
// ************************************************************************* //

#ifndef BOUNDARY_LIST_H
#define BOUNDARY_LIST_H
#include <siloobj_exports.h>

#include <silo.h>

#include "OnionPeel.h"


// ****************************************************************************
//  Class: BoundaryList
//
//  Purpose:
//      Keeps information about boundaries between materials.
//
//  Data Members:
//      nMaterialNumbers        -  The number of materials.
//      materialNumbers         -  The number associated with each material.
//      nShapes                 -  An array of size nMaterialNumbers that
//                                 contains the number of shapes for each
//                                 material.
//      totalShapes             -  The total number of shapes across all 
//                                 materials.  This is the sum of the nShapes
//                                 array.
//      shapeCnt                -  An array of size totalShapes.  It contains
//                                 the number with that shape for each shape
//                                 and each material.
//      shapeSize               -  The size of each shape.  An array of size
//                                 totalShapes.
//      nOppositeMaterial       -  An array of size totalShapes.  It contains
//                                 the number of materials opposite each
//                                 shape for each material. 
//      totalOpposite           -  The total number of material opposite
//                                 each of the shapes for each of the 
//                                 materials.  This is the sum of the 
//                                 nOppositeMaterial array.
//      oppositeMaterialCount   -  The number of shapes for a given material
//                                 opposite this shape for this material.
//      oppositeMaterialNumber  -  The corresponding material number for 
//                                 the oppositeMaterialCount array.
//      nodeList                -  ??? NEEDS HELP ???
//      
//  Programmer: Hank Childs
//  Creation:   January 7, 2000
//
// ****************************************************************************

class SILOOBJ_API BoundaryList
{
  public:
                 BoundaryList();
                ~BoundaryList();

    void         AddMaterial(DBmaterial *, OnionPeel *, DBzonelist *);
    void         SetName(char *);
    void         Write(DBfile *);

  private:
    int          nMaterialNumbers;
    int         *materialNumbers;
    int         *nShapes;
    int          totalShapes;
    int         *shapeCnt;
    int         *shapeSize;
    int         *nOppositeMaterial;
    int          totalOpposite;
    int         *oppositeMaterialCnt;
    int         *oppositeMaterialNumber;
    int         *nodeList;

    char        *name;

    // Constants
    static char * const   SILO_LOCATION;
    static int const      SILO_NUM_COMPONENTS;
    static char * const   SILO_TYPE;
    static char * const   SILO_N_MATERIAL_NUMBERS_NAME;
    static char * const   SILO_MATERIAL_NUMBERS_NAME;
    static char * const   SILO_N_SHAPES_NAME;
    static char * const   SILO_SHAPE_CNT_NAME;
    static char * const   SILO_SHAPE_SIZE_NAME;
    static char * const   SILO_N_OPPOSITE_MATERIAL_NAME;
    static char * const   SILO_OPPOSITE_MATERIAL_CNT_NAME;
    static char * const   SILO_OPPOSITE_MATERIAL_NUMBER_NAME;
    static char * const   SILO_NODE_LIST_NAME;
};


#endif


