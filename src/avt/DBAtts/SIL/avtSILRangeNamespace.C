// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
}


// ****************************************************************************
//  Method: avtSILRangeNamespace::Print
//
//  Purpose:
//      Prints out the subsets
//
//  Programmer: Hank Childs
//  Creation:   December 11, 2009
//
// ****************************************************************************

void
avtSILRangeNamespace::Print(ostream &out) const
{
    out << "Subsets: " << minRange << "-" << maxRange << endl;
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


// ****************************************************************************
//  Method: avtSILRangeNamespace::ContainsElement
//
//  Purpose:
//      Determine whether the element is contained in this namespace.
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 10:31:43 PST 2007
//
// ****************************************************************************

bool
avtSILRangeNamespace::ContainsElement(int e) const
{
    return (e >= minRange && e <= maxRange);
}





