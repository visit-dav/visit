// ************************************************************************* //
//                             avtGeneralQuery.h                             //
// ************************************************************************* //

#ifndef AVT_GENERAL_QUERY_H
#define AVT_GENERAL_QUERY_H

#include <query_exports.h>

#include <avtDataObjectQuery.h>


// ****************************************************************************
//  Class: avtGeneralQuery
//
//  Purpose:
//      A base class for "general" queries -- queries that are not specific
//      to data type.
//
//  Programmer: Hank Childs
//  Creation:   October 4, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class QUERY_API avtGeneralQuery : public avtDataObjectQuery
{
  public:
                               avtGeneralQuery();
    virtual                   ~avtGeneralQuery();

  protected:
    avtDataObject_p            input;
    
    void                       SetTypedInput(avtDataObject_p in)
                                     { input = in; };
    avtDataObject_p            GetInput(void)  { return input; };
};


#endif


