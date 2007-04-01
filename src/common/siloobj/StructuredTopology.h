// ************************************************************************* //
//                          StructuredTopology.h                             //
// ************************************************************************* //

#ifndef STRUCTURED_TOPOLOGY_H
#define STRUCTURED_TOPOLOGY_H
#include <siloobj_exports.h>

#include <visitstream.h>

#include <silo.h>


// ****************************************************************************
//  Class: StructuredTopology
//
//  Purpose:
//      Keeps information about a structured topology.
//
//  Data Members:
//      nDimensions  -  The number of dimensions.
//      dimensions   -  The dimensions of the structured blocks.  This is an
//                      array of size nDimensions*nDomains*2.
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

class SILOOBJ_API StructuredTopology
{
  public:
                    StructuredTopology();
    virtual        ~StructuredTopology();
   
    char           *GetName()  { return name; };

    void            PrintSelf(ostream &);
    void            Read(DBobject *, DBfile *);
    virtual void    Write(DBfile *);
   
  protected:
    int             nDimensions;
    float          *dimensions;

    int             nDomains;
    char           *name;

    // Class-scoped constants
  public:
    static char * const    NAME;
    static char * const    SILO_TYPE;
  protected:
    static char * const    SILO_NAME;
    static int    const    SILO_NUM_COMPONENTS;
    static char * const    SILO_N_DIMENSIONS_NAME;
    static char * const    SILO_DIMENSIONS_NAME;
};


#endif


