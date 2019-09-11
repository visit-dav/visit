// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtStreamingGhostGenerator.h                      //
// ************************************************************************* //

#ifndef AVT_STREAMING_GHOST_GENERATOR_H
#define AVT_STREAMING_GHOST_GENERATOR_H

#include <database_exports.h>
#include <cstddef>
#include <vector>

#include <avtGhostData.h>

class vtkDataSet;
class vtkDataArray;
class avtMixedVariable;
class avtMaterial;

// ****************************************************************************
//  Class:  avtStreamingGhostGenerator
//
//  Purpose:
//      An abstract interface for creating ghost data in a streaming setting.
//
//  Programmer:  Hank Childs
//  Creation:    February 10, 2008
//
// ****************************************************************************

class DATABASE_API avtStreamingGhostGenerator
{
  public:
                          avtStreamingGhostGenerator();
    virtual              ~avtStreamingGhostGenerator();

    static void           Destruct(void *);
    static int            LBGetNextDomain();

    virtual int           GetNextDomain(void) = 0;
    virtual vtkDataSet   *StreamDataset(vtkDataSet *) = 0;

  protected:
    static avtStreamingGhostGenerator *instance;
};

#endif
