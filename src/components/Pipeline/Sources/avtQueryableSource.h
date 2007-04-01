// ************************************************************************* //
//                             avtQueryableSourcy.h                          //
// ************************************************************************* //

#ifndef AVT_QUERYABLE_SOURCE_H
#define AVT_QUERYABLE_SOURCE_H
#include <pipeline_exports.h>

#include <avtDataObjectSource.h>

class PickAttributes;


// ****************************************************************************
//  Method: avtQueryableSource
//
//  Purpose:
//      A queryable source is one that is queryable.  All terminating sources
//      are considered queryable, ensuring that every pipeline has a
//      queryable source.
//
//  Programmer: Hank Childs
//  Creation:   July 28, 2003
//
//  Modifications:
//    Kathleen Bonnell, Wed Nov 12 18:26:21 PST 2003
//    Add virtual method 'FindElementForPoint'.
//
//    Kathleen Bonnell, Mon Dec 22 14:48:57 PST 2003 
//    Add virtual method 'GetDomainName'.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Kathleen Bonnell, Tue May 25 16:16:25 PDT 2004 
//    Add virtual method 'QueryZoneCenter'.
//
// ****************************************************************************

class PIPELINE_API avtQueryableSource : virtual public avtDataObjectSource
{
  public:
                                  avtQueryableSource();
    virtual                      ~avtQueryableSource();

    virtual avtQueryableSource   *GetQueryableSource(void) { return this; };
    virtual void                  Query(PickAttributes *) = 0;
    virtual bool                  FindElementForPoint(const char*, const int, 
                                    const int, const char*, float[3], int &)=0;
    virtual void                  GetDomainName(const std::string&, const int, 
                                    const int, std::string&)=0;
    virtual bool                  QueryZoneCenter(const std::string&, const int, 
                                    const int, const int, float[3])=0;
};


#endif


