// ************************************************************************* //
//                      UnstructuredTopology_Prep.h                          //
// ************************************************************************* //

#ifndef UNSTRUCTURED_TOPOLOGY_PREP_H
#define UNSTRUCTURED_TOPOLOGY_PREP_H

#include <silo.h>

#include <UnstructuredTopology.h>
#include <Value_Prep.h>


// ****************************************************************************
//  Class: UnstructuredTopology_Prep
//
//  Purpose:
//      A derived type of UnstructuredTopology that is meant exclusively for
//      meshtvprep.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

class UnstructuredTopology_Prep : public UnstructuredTopology
{
  public:
                     UnstructuredTopology_Prep();
    virtual         ~UnstructuredTopology_Prep();
  
    void             DetermineSize(int, DBfile *, char *);

    void             SetName(char *);
    void             SetDomains(int);

    void             Consolidate(void);
    void             WrapUp(void);
    virtual void     Write(DBfile *);
    void             WriteArrays(DBfile *, DBucdmesh *, int);
   
  protected:
    int              nDomains;

    Value_Prep      *aliasedNodeListValue;
    Value_Prep      *aliasedShapeCountValue;
    Value_Prep      *aliasedShapeTypeValue;
    Value_Prep      *aliasedShapeSizeValue;

    // Protected methods
    virtual void     CreateValues();

    // Class-scoped constants
    static char * const   SILO_LOCATION;
};


#endif


