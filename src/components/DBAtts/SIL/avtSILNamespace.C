// ************************************************************************* //
//                               avtSILNamespace.C                           //
// ************************************************************************* //

#include <avtSILEnumeratedNamespace.h>
#include <avtSILRangeNamespace.h>

#include <NamespaceAttributes.h>

#include <DebugStream.h>


using  std::vector;


// ****************************************************************************
//  Method: avtSILNamespace::GetNamespace
//
//  Purpose:
//      Takes in a NamespaceAttributes object and creates the correct derived
//      type of avtSILNamespace.
//
//  Arguments:
//      atts      The attribute subject.
//
//  Returns:      The new namespace object.
//
//  Programmer:   Hank Childs
//  Creation:     March 29, 2001
//
// ****************************************************************************

avtSILNamespace *
avtSILNamespace::GetNamespace(const NamespaceAttributes *atts)
{
    int type = atts->GetType();
    if (type == 0)
    {
        return new avtSILEnumeratedNamespace(atts->GetSubsets());
    }
    else if (type == 1)
    {
        const vector<int> &ss = atts->GetSubsets();
        int  subset = ss[0];
        int  min = atts->GetMin();
        int  max = atts->GetMax();
        return new avtSILRangeNamespace(subset, min, max);
    }

    //
    // Don't know what to do, so flag it.
    //
    debug1 << "Invalid namespace type " << type << endl;
    return NULL;
}


