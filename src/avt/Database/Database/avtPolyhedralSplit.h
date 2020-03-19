// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
#ifndef AVT_POLYHEDRAL_SPLIT_H
#define AVT_POLYHEDRAL_SPLIT_H

#include <vectortypes.h>
#include "database_exports.h"

class vtkDataArray;

// ****************************************************************************
// Class: avtPolyhedralSplit
//
// Purpose:
//   This class contains a list of polyhedral cells that were split into zoo
//   cells and their split count. The class also contains methods for expanding
//   data arrays to account for the cell splitting.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 11:29:46 PST 2010
//
// Modifications:
//   Brad Whitlock, Tue Oct 26 16:23:45 PDT 2010
//   I turned polyhedralSplit into a vector so we could add polyhedral split
//   information as we discover it.
//
//   Cyrus Harrison, Tue Oct  9 14:12:12 PDT 2012
//   Moved here from the SimV2 reader, since other db readers need
//   to use this functionality.
//
// ****************************************************************************

class DATABASE_API avtPolyhedralSplit
{
public:
            avtPolyhedralSplit();
           ~avtPolyhedralSplit();

    static void Destruct(void *);

    void AppendCellSplits(int cellid, int nsplits);

    void AppendPolyhedralNode(int);

    vtkDataArray *ExpandDataArray(vtkDataArray *input, bool zoneCent,
                                  bool averageNodes=true) const;
    vtkDataArray *CreateOriginalCells(int domain, int normalCellCount) const;

private:
    intVector polyhedralSplit;
    intVector nodesForPolyhedralCells;
};

#endif
