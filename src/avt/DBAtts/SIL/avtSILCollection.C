// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtSILCollection.C                            //
// ************************************************************************* //

#include <avtSILCollection.h>

#include <vector>

#include <avtSILNamespace.h>

#include <ImproperUseException.h>


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
//  Method: avtSILCollection destructor
//
//  Programmer: Mark C. Miller, Wed Aug 22 08:33:09 PDT 2012
//
//  Added private operator= and copy constructor to prevent inadvertent use.
//  The class has a data member that points to heap memory but these
//  methods were never implemented and it was easier to simply prevent their
//  accidental use than to figure out how best to implement them.

// ****************************************************************************
avtSILCollection::avtSILCollection(const avtSILCollection&)
{
    EXCEPTION0(ImproperUseException);
}

const avtSILCollection&
avtSILCollection::operator=(const avtSILCollection&)
{
    EXCEPTION0(ImproperUseException);
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
//    Jeremy Meredith, Fri Aug 25 17:16:38 EDT 2006
//    Added enumerated scalars.
//
//    Hank Childs, Fri Dec 11 11:37:48 PST 2009
//    Push printing off the name spaces ... that way we don't have to
//    enumerate all the values.
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
      case SIL_ENUMERATION:
        roleS = "enumeration";
        break;
      case SIL_USERD:
        roleS = "user defined";
        break;
    }
    out << roleS.c_str() << endl;

    out << "\t\tSuperset is " << supersetIndex << endl;

    out << "\t\t";
    subsets->Print(out);
}


// ****************************************************************************
//  Method: avtSILCollection::ContainsElement
//
//  Purpose:
//      Returns true if the element is contained in this collection.
//
//  Programmer: Dave Bremer
//  Creation:   Thu Dec 20 14:23:29 PST 2007
//
// ****************************************************************************

bool
avtSILCollection::ContainsElement(int e) const
{
    return (subsets->ContainsElement(e));
}
