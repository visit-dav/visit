// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
    sequentialElems = true;
    size_t ii;
    for (ii = 1; ii < els.size(); ii++)
    {
        if (els[ii-1]+1 != els[ii])
        {
            sequentialElems = false;
            break;
        }
    }
    sortedElems = true;
    for (ii = 1; ii < els.size(); ii++)
    {
        if (els[ii-1] > els[ii])
        {
            sortedElems = false;
            break;
        }
    }
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



// ****************************************************************************
//  Method: avtSILEnumeratedNamespace::ContainsElement
//
//  Purpose:
//      Determine whether the element is contained in this namespace.  This 
//      class detects the common case of a sequence of elements, and uses that
//      info to speed up the query.
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 10:31:43 PST 2007
//
// ****************************************************************************

bool
avtSILEnumeratedNamespace::ContainsElement(int e) const
{
    if (sequentialElems)
    {
        return (e >= elements[0] && e <= elements[elements.size()-1]);
    }
    else if (sortedElems)
    {
        int min = 0, max = static_cast<int>(elements.size())-1;
        int mid = (max+min)/2;
        while (min <= max)
        {
            if (elements[mid] == e)
            {
                return true;
            }
            else if (elements[mid] > e)
            {
                max = mid-1;
                mid = (max+min)/2;
            }
            else
            {
                min = mid+1;
                mid = (max+min)/2;
            }
        }
        return false;
    }
    else
    {
        for (size_t ii = 0; ii < elements.size(); ii++)
        {
            if (elements[ii] == e)
                return true;
        }
    }
    return false;
}


// ****************************************************************************
//  Method: avtSILEnumerateNamespace::Print
//
//  Purpose:
//      Prints out the subsets
//
//  Programmer: Hank Childs
//  Creation:   December 11, 2009
//
// ****************************************************************************

void
avtSILEnumeratedNamespace::Print(ostream &out) const
{
    std::vector<int>::const_iterator it;
    out << "Subsets: ";
    for (it = elements.begin() ; it != elements.end(); it++)
    {
        out << *it << ", ";
    }
    out << endl;
}


