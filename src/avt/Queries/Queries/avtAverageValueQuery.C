// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtAverageValueQuery.C                         //
// ************************************************************************* //

#include <avtAverageValueQuery.h>

#include <DebugStream.h>


using     std::string;


// ****************************************************************************
//  Method: avtAverageValueQuery constructor
//
//  Programmer: Hank Childs 
//  Creation:   May 12, 2011
//
// ****************************************************************************

avtAverageValueQuery::avtAverageValueQuery() 
    : avtWeightedVariableSummationQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtAverageValueQuery destructor
//
//  Programmer: Hank Childs 
//  Creation:   May 12, 2011
//
// ****************************************************************************

avtAverageValueQuery::~avtAverageValueQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtAverageValueQuery::CreateVariable
//
//  Purpose:
//     Creates the variable for the summation.
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2011
//
// ****************************************************************************

avtDataObject_p
avtAverageValueQuery::CreateVariable(avtDataObject_p inData)
{
    return inData;
}


