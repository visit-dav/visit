// ************************************************************************* //
//                               Value_Prep.h                                //
// ************************************************************************* //

#ifndef VALUE_PREP_H
#define VALUE_PREP_H

#include <iostream.h>
#include <silo.h>

#include <MeshtvPrepLib.h>
#include <Value.h>


// ****************************************************************************
//  Class: Value_Prep
// 
//  Purpose:
//      A derived type of Value, this is meant exclusively for meshtvprep.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

class Value_Prep : public Value
{
  public:
                   Value_Prep();
    virtual       ~Value_Prep();
    
    void           AddDomain(int *, int);

    int            GetSize(void) { return totalSize; };
    void           SetNDomains(int);
    void           SetNVals(int);
    int            GetNVals(void)    { return nVals; };
    bool           IsSetNVals(void)  { return setNVals; };
    void           SetFieldName(char *);
    void           SetResource(RESOURCE_e);
    static void    SetFileBase(char *);
    static void    SetFileNum(void);
    static void    SetNFiles(int);

    void           Consolidate(void);
    void           WrapUp(void);
    virtual void   Write(DBfile *);

    void           UpdateLength(int, int *);
    void           WriteArray(DBfile *, void **, int, int);

  protected:
    int           *files;
 
    int            totalSize;
    int           *sizeForFile;
    int            thisProcsOffset;
    bool           setNames;
    bool           setNVals;

    RESOURCE_e     resource;

    static char   *fileBase;
    static int     fileNum;
    static int     nFiles;

    // Protected Methods
    void           SetNames();
    inline int     Index(int, int);
   
    // Class-Scoped Constants
    static char * const SILO_LOCATION;
};


#endif


