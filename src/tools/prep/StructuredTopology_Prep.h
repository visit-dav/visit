// ************************************************************************* //
//                       StructuredTopology_Prep.h                           //
// ************************************************************************* //

#ifndef STRUCTURED_TOPOLOGY_PREP_H
#define STRUCTURED_TOPOLOGY_PREP_H

#include <silo.h>

#include <StructuredTopology.h>


// ****************************************************************************
//  Class: StructuredTopology_Prep
//
//  Purpose:
//      A derived type of StructuredTopology meant exclusively for meshtvprep.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2000
//
// ****************************************************************************

class StructuredTopology_Prep : public StructuredTopology
{
  public:
                    StructuredTopology_Prep();
    virtual        ~StructuredTopology_Prep();
   
    void            ReadMesh(DBquadmesh *, int);

    void            SetName(char *);
    void            SetDomains(int);

    void            Consolidate(void);
    void            WrapUp(void);
    virtual void    Write(DBfile *);
   
  private:
    int     dimensionsSize;
    bool    readDimension;

    // Private Methods
    void    SetDimension(int);

    // Class-scoped constants
    static char * const    SILO_LOCATION;
};


#endif


