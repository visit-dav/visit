// ************************************************************************* //
//                            avtSILRangeNamespace.C                         //
// ************************************************************************* //

#include <avtSILRangeNamespace.h>

#include <NamespaceAttributes.h>


using std::vector;


// ****************************************************************************
//  Method: avtSILRangeNamespace constructor
//
//  Arguments:
//      s       A set has been put in the SIL as the union of all the sets this
//              namespace is over for conceptual purposes.  This is the index
//              of that set.
//      min     The minimum in the range.
//      max     The maximum in the range.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
// ****************************************************************************

avtSILRangeNamespace::avtSILRangeNamespace(int s, int min, int max)
{
    set = s;
    minRange = min;
    maxRange = max;

    //
    // We need to return a reference to a vector, so create one with the object
    // that contains the one set we will point at.
    //
    setAsElementList.push_back(set);
}


// ****************************************************************************
//  Method: avtSILRangeNamespace::GetAllElements
//
//  Purpose:
//      Returns the element list for this namespace.  There are actually
//      elements from minRange to maxRange, but they are compressed into one
//      set for conceptual purposes.  That set is the data member 'set'.  Since
//      we need to return a reference to a vector, there is a data member
//      kept around just to return -- setAsElementList.
//
//  Returns:     The element list for this namespace.
//
//  Programmer:  Hank Childs
//  Creation:    March 9, 2001
//
// ****************************************************************************

const vector<int> &
avtSILRangeNamespace::GetAllElements(void) const
{
    return setAsElementList;
}


// ****************************************************************************
//  Method: avtSILRangeNamespace::GetAttributes
//
//  Purpose:
//      Gets the namespace attributes when the namespace is for ranges.  Note
//      that it puts '1' in as the type and this type must be maintained in the
//      base class.  Every derived type should have its own unique number.
//
//  Returns:    The new attributes, the calling function owns this.
//
//  Programmer: Hank Childs
//  Creation:   March 29, 2001
//
// ****************************************************************************

NamespaceAttributes *
avtSILRangeNamespace::GetAttributes(void) const
{
    NamespaceAttributes *rv = new NamespaceAttributes;
    rv->SetType(1);
    vector<int> ss;
    ss.push_back(set);
    rv->SetSubsets(ss);
    rv->SetMin(minRange);
    rv->SetMax(maxRange);

    return rv;
}


