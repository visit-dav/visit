// ************************************************************************* //
//                             avtQueryableSource.h                          //
// ************************************************************************* //

#ifndef AVT_QUERYABLE_SOURCE_H
#define AVT_QUERYABLE_SOURCE_H


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
// ****************************************************************************

class avtQueryableSource : virtual public avtDataObjectSource
{
  public:
                                  avtQueryableSource() {;};
    virtual                      ~avtQueryableSource() {;};

    virtual avtQueryableSource   *GetQueryableSource(void) { return this; };
    virtual void                  Query(PickAttributes *) = 0;
};


#endif


