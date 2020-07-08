// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtGhostNodeGenerator.h                         //
// ************************************************************************* //

#ifndef AVT_GHOST_NODE_GENERATOR_H
#define AVT_GHOST_NODE_GENERATOR_H

#include <database_exports.h>

#include <string>
#include <vector>

class avtDatasetCollection;

// ****************************************************************************
//  Class: avtGhostNodeGenerator
//
//  Purpose:
//      It generates ghost nodes for a collection of structured grids.
//
//      It creates a list of the extents for all the faces for all the grids
//      on a given processor and then does an all to all exchange so that
//      each processor has the extents for all the faces for all the blocks.
//      This in theory could use a lot of memory so if the total number
//      of faces will exceed 20 million it doesn't execute.
//
//      There are some optimizations where it doesn't check faces between
//      blocks that don't have any overlap or check ndividual faces against
//      blocks that don't wholely contain the face.
//
//      The high level idea for the algorithm came from Matt Larsen.
//
//  Programmer: Eric Brugger
//  Creation:   May 28, 2020
//
//  Modifications:
//
// ****************************************************************************

class DATABASE_API avtGhostNodeGenerator
{
  public:
                                    avtGhostNodeGenerator();
    virtual                        ~avtGhostNodeGenerator();

    bool                            CreateGhosts(avtDatasetCollection &ds);

  private:
    bool                            IsValid(avtDatasetCollection &ds);
};


#endif
