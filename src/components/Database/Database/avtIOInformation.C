// ************************************************************************* //
//                              avtIOInformation.C                           //
// ************************************************************************* //

#include <avtIOInformation.h>


// ****************************************************************************
//  Method: avtIOInformation constructor
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2001
//
// ****************************************************************************

avtIOInformation::avtIOInformation()
{
    nDomains = 0;
}


// ****************************************************************************
//  Method: avtIOInformation::AddHints
//
//  Purpose:
//      Assigns the hints to use.
//
//  Arguments:
//      h     The new hints.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2001
//
// ****************************************************************************

void
avtIOInformation::AddHints(HintList &h)
{
    hints = h;
}

// ****************************************************************************
//  Method: avtIOInformation::SetNDomains
//
//  Purpose:
//      Sets the number of domains.
//
//  Arguments:
//      n       the number of domains
//
//  Programmer: Jeremy Meredith
//  Creation:   September 18, 2001
//
// ****************************************************************************

void
avtIOInformation::SetNDomains(int n)
{
    nDomains = n;
}


