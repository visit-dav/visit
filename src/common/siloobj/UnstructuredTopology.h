// ************************************************************************* //
//                         UnstructuredTopology.h                            //
// ************************************************************************* //

#ifndef UNSTRUCTURED_TOPOLOGY_H
#define UNSTRUCTURED_TOPOLOGY_H
#include <siloobj_exports.h>

#include <visitstream.h>
#include <silo.h>

#include <SiloObjLib.h>
#include <Value.h>


// ****************************************************************************
//  Class: UnstructuredTopology
//
//  Purpose:
//      Keeps information about an unstructured topology.
//
//  Data Members:
//      nodeList    -  The name of the node list value object.
//      shapeCount  -  The name of the shape count value object.
//      shapeType   -  The name of the shape type value object.
//      shapeSize   -  The name of the shape size value object.
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

class SILOOBJ_API UnstructuredTopology
{
  public:
                     UnstructuredTopology();
                     UnstructuredTopology(FOR_DERIVED_TYPES_e);
    virtual         ~UnstructuredTopology();
  
    char            *GetName(void)   { return name; };

    void             PrintSelf(ostream &);
    void             Read(DBobject *, DBfile *);
    virtual void     Write(DBfile *);
   
  protected:
    char            *nodeList;
    char            *shapeCount;
    char            *shapeType;
    char            *shapeSize;

    char            *name;

    Value           *nodeListValue;
    Value           *shapeCountValue;
    Value           *shapeTypeValue;
    Value           *shapeSizeValue;

    // Protected Methods
    void             CreateValues(void);
    void             RealConstructor(void);
    
    // Class-scoped constants
  public:
    static char * const   NAME;
    static char * const   SILO_TYPE;
  protected:
    static char * const   SILO_NAME;
    static int    const   SILO_NUM_COMPONENTS;
    static char * const   SILO_NODE_LIST_NAME;
    static char * const   SILO_SHAPE_COUNT_NAME;
    static char * const   SILO_SHAPE_TYPE_NAME;
    static char * const   SILO_SHAPE_SIZE_NAME;
};


#endif


