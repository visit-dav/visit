// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtTimeLoopCollectorFilter.C                         //
// ************************************************************************* //

#include <avtTimeLoopCollectorFilter.h>

#include <avtTimeLoopFilter.h>


// ****************************************************************************
//  Method: avtTimeLoopCollectorFilter constructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 22, 2008
//
// ****************************************************************************

avtTimeLoopCollectorFilter::avtTimeLoopCollectorFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtTimeLoopCollectorFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 22, 2008 
//
// ****************************************************************************

avtTimeLoopCollectorFilter::~avtTimeLoopCollectorFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtTimeLoopCollectorFilter::CreateFinalOutput
//
//  Purpose:  
//      Called when all of the time slices have been iterated over.  This then
//      turns around and calls the derived types "ExecuteAllTimesteps" method.
//
//  Programmer: Hank Childs
//  Creation:   January 22, 2008
//
// ****************************************************************************

void
avtTimeLoopCollectorFilter::CreateFinalOutput()
{
    avtDataTree_p output = ExecuteAllTimesteps(trees);
    SetOutputDataTree(output);
    trees.clear();
}


// ****************************************************************************
//  Method: avtTimeLoopCollectorFilter::Execute
//
//  Purpose:  
//      Called when a data set is ready to be executed on.  This stores the
//      data set for later use.
//
//  Programmer: Hank Childs
//  Creation:   January 22, 2008
//
// ****************************************************************************

void
avtTimeLoopCollectorFilter::Execute()
{
    trees.push_back(GetInputDataTree());
}


// ****************************************************************************
//  Method: avtTimeLoopCollectorFilter::ReleaseData
//
//  Purpose:
//      Makes the output release any data that it has as a memory savings.
//
//  Programmer: Hank Childs
//  Creation:   January 22, 2008
//
// ****************************************************************************

void
avtTimeLoopCollectorFilter::ReleaseData(void)
{
    trees.clear();
    avtFilter::ReleaseData();    
}
