// ************************************************************************* //
//                         avtSILEnumeratedNamespace.h                       //
// ************************************************************************* //

#ifndef AVT_SIL_ENUMERATED_NAMESPACE_H
#define AVT_SIL_ENUMERATED_NAMESPACE_H
#include <dbatts_exports.h>


#include <avtSILNamespace.h>


// ****************************************************************************
//  Class: avtSILEnumeratedNamespace
//
//  Purpose:
//      This is a concrete type of namespace.  This is used for when a
//      collections maps onto a number of subsets that can be reasonably
//      enumerated.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
// ****************************************************************************

class DBATTS_API avtSILEnumeratedNamespace : public avtSILNamespace
{
  public:
                        avtSILEnumeratedNamespace(const std::vector<int> &);
    virtual            ~avtSILEnumeratedNamespace() {;};

    virtual const std::vector<int>  &GetAllElements(void) const;
    virtual NamespaceAttributes     *GetAttributes(void) const;

  protected:
    std::vector<int>    elements;
};


#endif


