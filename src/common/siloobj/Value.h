// ************************************************************************* //
//                                 Value.h                                   //
// ************************************************************************* //

#ifndef VALUE_H
#define VALUE_H
#include <siloobj_exports.h>

#include <visitstream.h>
#include <silo.h>


// ****************************************************************************
//  Class: Value
// 
//  Purpose:
//      Keeps information about a value.
//
//  Data Members:
//      nDomains       -  The number of domains.
//      nVals          -  The number of variables in the vector this Value 
//                        represents.
//      totalEntries   -  The total number of elements for some arrays.  This 
//                        is nDomains*nVals.
//      entryNames     -  List of array names for the values for each domain. 
//                        The array is of size nDomains.
//      offsets        -  List of offsets into the values arrays for each 
//                        domain.  The array is of size totalEntries.
//      lengths        -  List of lengths of values for each domain.  The array
//                        is of size totalEntries.
//      files          -  The file number that each domain should go into.
//                        The array is of size nDomains.
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

class SILOOBJ_API Value
{
  public:
                      Value();
    virtual          ~Value();
    
    char             *GetName() { return name; };
   
    void              PrintSelf(ostream &);

    void              Read(DBobject *, DBfile *);
    virtual void      Write(DBfile *);

  protected:
    int               nDomains;
    int               nVals;
    int               totalEntries;

    char            **entryNames;
    int              *offsets;
    int              *lengths;
 
    char             *name;
   
    // Class-Scoped Constants
  public:
    static char * const NAME;
    static char * const SILO_TYPE;
  protected:
    static char * const ARRAY_STRING;
    static int    const SILO_NUM_COMPONENTS;
    static char * const SILO_OBJ_NAME;

    static char * const SILO_N_DOMAINS_NAME;
    static char * const SILO_N_VALS_NAME;
    static char * const SILO_LENGTHS_NAME;
    static char * const SILO_ENTRY_NAMES_NAME;
    static char * const SILO_OFFSETS_NAME;
};


#endif


