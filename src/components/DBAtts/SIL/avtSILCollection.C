// ************************************************************************* //
//                             avtSILCollection.C                            //
// ************************************************************************* //

#include <avtSILCollection.h>

#include <vector>

#include <avtSILNamespace.h>


using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtSILCollection constructor
//
//  Arguments:
//      cat         The name of the category.
//      r           The role of the category.
//      si          The superset index.
//      ns          The namespace that has the subset relations.  This object
//                  will be owned by the collection and deleted when the
//                  collection is deleted.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
// ****************************************************************************

avtSILCollection::avtSILCollection(string cat, SILCategoryRole r, int si,
                                   avtSILNamespace *ns)
{
    category      = cat;
    role          = r;
    supersetIndex = si;
    subsets       = ns;
}


// ****************************************************************************
//  Method: avtSILCollection destructor
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
// ****************************************************************************

avtSILCollection::~avtSILCollection()
{
    if (subsets != NULL)
    {
        delete subsets;
        subsets = NULL;
    }
}


// ****************************************************************************
//  Method: avtSILCollection::GetSubsetList
//
//  Purpose:
//      Get the list of subsets.  This goes through other routines, but
//      streamlines their use to minimize the amount clients need to know
//      about this class.
//
//  Returns:    A list of subsets.  These are the actual sets for an enumerated
//              namespace, and a placeholder for a range namespace.
// 
//  Programmer: Hank Childs
//  Creation:   May 21, 2001
//
// ****************************************************************************

const vector<int> &
avtSILCollection::GetSubsetList(void) const
{
    return subsets->GetAllElements();
}


// ****************************************************************************
//  Method: avtSILCollection::Print
//
//  Purpose:
//      Prints out a collection -- for debugging purposed only.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep 18 10:20:35 PDT 2001
//    Added case for boundary.
//
//    Jeremy Meredith, Fri Dec 14 16:25:54 PST 2001
//    Added species.
//
// ****************************************************************************

void
avtSILCollection::Print(ostream &out) const
{
    out << "\tCollection for category = " << category.c_str() << ", role = ";
    string roleS = "";
    switch (role)
    {
      case SIL_TOPOLOGY:
        roleS = "topology";
        break;
      case SIL_PROCESSOR:
        roleS = "processor";
        break;
      case SIL_BLOCK:
        roleS = "block";
        break;
      case SIL_DOMAIN:
        roleS = "domain";
        break;
      case SIL_ASSEMBLY:
        roleS = "assembly";
        break;
      case SIL_MATERIAL:
        roleS = "material";
        break;
      case SIL_BOUNDARY:
        roleS = "boundary";
        break;
      case SIL_SPECIES:
        roleS = "species";
        break;
      case SIL_USERD:
        roleS = "user defined";
        break;
    }
    out << roleS.c_str() << endl;

    out << "\t\tSuperset is " << supersetIndex << endl;

    const vector<int> &els = subsets->GetAllElements();
    std::vector<int>::const_iterator it;
    out << "\t\t";
    for (it = els.begin() ; it != els.end(); it++)
    {
        out << *it << ", ";
    }
    out << endl;
}


