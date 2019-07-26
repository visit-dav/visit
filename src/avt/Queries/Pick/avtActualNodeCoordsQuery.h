// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtActualNodeCoordsQuery.h                          //
// ************************************************************************* //

#ifndef AVT_ACTUAL_NODECOORDS_QUERY_H
#define AVT_ACTUAL_NODECOORDS_QUERY_H
#include <query_exports.h>

#include <avtActualCoordsQuery.h>



// ****************************************************************************
//  Class: avtActualNodeCoordsQuery
//
//  Purpose:
//      This query determines the actual coordinates for a given node.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 18, 2004 
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtActualNodeCoordsQuery : public avtActualCoordsQuery
{
  public:
                                    avtActualNodeCoordsQuery();
    virtual                        ~avtActualNodeCoordsQuery();

    virtual const char             *GetType(void)
                                             { return "avtActualNodeCoordsQuery"; };
    virtual const char             *GetDescription(void)
                                             { return "Locating node coords."; };

  protected:
    virtual void                    Execute(vtkDataSet *, const int);
};


#endif


