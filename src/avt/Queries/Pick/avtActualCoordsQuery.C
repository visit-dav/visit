// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtActualCoordsQuery.C                          //
// ************************************************************************* //

#include <avtActualCoordsQuery.h>

#include <avtParallel.h>


// ****************************************************************************
//  Method: avtActualCoordsQuery constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtActualCoordsQuery::avtActualCoordsQuery()
{
    actualId = -1;
    actualCoords[0] = 0.;
    actualCoords[1] = 0.;
    actualCoords[2] = 0.;
}


// ****************************************************************************
//  Method: avtActualCoordsQuery destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 2, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtActualCoordsQuery::~avtActualCoordsQuery()
{
}

// ****************************************************************************
//  Method: avtActualCoordsQuery::SetPickAtts
//
//  Purpose:
//      Sets the pickAtts to the passed values. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2004 
//
// ****************************************************************************

void
avtActualCoordsQuery::SetPickAtts(const PickAttributes *pa)
{
    pickAtts =  *pa;
}


// ****************************************************************************
//  Method: avtActualCoordsQuery::GetPickAtts
//
//  Purpose:
//    Retrieve the PickAttributes being used in this query.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2004
//
// ****************************************************************************

const PickAttributes *
avtActualCoordsQuery::GetPickAtts() 
{
    return &pickAtts; 
}



// ****************************************************************************
//  Method: avtPickQuery::PreExecute
//
//  Purpose:
//    This is called before any of the domains are executed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2004 
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
// ****************************************************************************

void
avtActualCoordsQuery::PreExecute()
{
    avtDatasetQuery::PreExecute();

    actualId = -1;
    actualCoords[0] = 0.;
    actualCoords[1] = 0.;
    actualCoords[2] = 0.;
}


// ****************************************************************************
//  Method: avtPickQuery::PostExecute
//
//  Purpose:
//    This is called after all of the domains are executed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 2, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtActualCoordsQuery::PostExecute()
{
    if (ThisProcessorHasMaximumValue(actualId) && actualId != -1)
    {
        pickAtts.SetPickPoint(actualCoords);
    }
}
