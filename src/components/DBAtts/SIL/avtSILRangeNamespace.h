// ************************************************************************* //
//                         avtSILRangeNamespace.h                            //
// ************************************************************************* //

#ifndef AVT_SIL_RANGE_NAMESPACE_H
#define AVT_SIL_RANGE_NAMESPACE_H
#include <dbatts_exports.h>


#include <avtSILNamespace.h>


// ****************************************************************************
//  Class: avtSILRangeNamespace
//
//  Purpose:
//      This is a concrete type of namespace.  This is used for when a
//      collection's maps onto a number of subsets that is problem size and
//      they can only be represented through a range.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
// ****************************************************************************

class DBATTS_API avtSILRangeNamespace : public avtSILNamespace
{
  public:
                                     avtSILRangeNamespace(int set, int min,
                                                          int max);
    virtual                         ~avtSILRangeNamespace() {;};

    virtual const std::vector<int>  &GetAllElements(void) const;

    virtual NamespaceAttributes     *GetAttributes(void) const;

  protected:
    int                              set;
    int                              minRange;
    int                              maxRange;
    std::vector<int>                 setAsElementList;
};


#endif


