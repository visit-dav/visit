// ************************************************************************* //
//                        avtSILEnumeratedNamespace.C                        //
// ************************************************************************* //

#include <avtSILEnumeratedNamespace.h>

#include <NamespaceAttributes.h>


using std::vector;


// ****************************************************************************
//  Method: avtSILEnumeratedNamespace constructor
//
//  Arguments:
//      els     The element list to copy.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
// ****************************************************************************

avtSILEnumeratedNamespace::avtSILEnumeratedNamespace(const vector<int> &els)
{
    elements = els;
}


// ****************************************************************************
//  Method: avtSILEnumeratedNamespace::GetAllElements
//
//  Purpose:
//      Returns the element list for this namespace.  Since this is an
//      enumerated namespace, this is exactly the elements we have previously
//      enumerated.
//
//  Returns:     The element list.
//
//  Programmer:  Hank Childs
//  Creation:    March 9, 2001
//
// ****************************************************************************

const vector<int> &
avtSILEnumeratedNamespace::GetAllElements(void) const
{
    return elements;
}


// ****************************************************************************
//  Method: avtSILEnumeratedNamespace::GetAttributes
//
//  Purpose:
//      Gets the namespace attributes when the namespace is enumerated.  Note
//      that it puts '0' in as the type and this type must be maintained in the
//      base class.  Every derived type should have its own unique number.
//
//  Returns:    The new attributes, the calling function owns this.
//
//  Programmer: Hank Childs
//  Creation:   March 29, 2001
//
// ****************************************************************************

NamespaceAttributes *
avtSILEnumeratedNamespace::GetAttributes(void) const
{
    NamespaceAttributes *rv = new NamespaceAttributes;
    rv->SetType(0);
    rv->SetSubsets(elements);

    return rv;
}


