// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <cstddef>

#include <avtStreamingGhostGenerator.h>


avtStreamingGhostGenerator *avtStreamingGhostGenerator::instance = NULL;


// ****************************************************************************
//  Constructor:  avtStreamingGhostGenerator::avtStreamingGhostGenerator
//
//  Programmer:  Hank Childs
//  Creation:    February 10, 2008
//
// ****************************************************************************

avtStreamingGhostGenerator::avtStreamingGhostGenerator()
{
    instance = this;
}


// ****************************************************************************
//  Destructor:  avtStreamingGhostGenerator::~avtStreamingGhostGenerator
//
//  Programmer:  Hank Childs
//  Creation:    February 10, 2008
//
// ****************************************************************************

avtStreamingGhostGenerator::~avtStreamingGhostGenerator()
{
    if (instance == this)
        instance = NULL;
}


// ****************************************************************************
//  Method: Destruct
//
//  Purpose:
//      A static method that makes ref_ptrs easier to deal with.
//
//  Programmer: Hank Childs
//  Creation:   February 10, 2008
//
// ****************************************************************************

void
avtStreamingGhostGenerator::Destruct(void *p)
{
    avtStreamingGhostGenerator *sgg = (avtStreamingGhostGenerator *) p;
    delete sgg;
}


// ****************************************************************************
//  Method: LBGetNextDomain
//
//  Purpose:
//      A static method that allows the load balancer to get the next domain.
//
//  Programmer: Hank Childs
//  Creation:   February 10, 2008
//
// ****************************************************************************

int
avtStreamingGhostGenerator::LBGetNextDomain(void)
{
    // Note that this assumes there is only one instance of a sgg.
    if (instance == NULL)
    {
        return -1;
    }
    return instance->GetNextDomain();
}


