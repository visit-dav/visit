// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtNodeCoordsQuery.h                             //
// ************************************************************************* //

#ifndef AVT_NODECOORDS_QUERY_H
#define AVT_NODECOORDS_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>


// ****************************************************************************
//  Class: avtNodeCoordsQuery
//
//  Purpose:
//      This is a dataset query that returns the coordintes of a node. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Dec 16 17:16:33 PST 2004
//    Add FindLocalCoord and FindGlobalCoord.
//
//    Kathleen Biagas, Mon Jun 20 12:33:33 PDT 2011
//    Add SetInputParams, domain, element, useGlobalId.
//
//    Kathleen Biagas, Fri Jul 15 16:27:06 PDT 2011
//    Add GetDefaultInputParams.
//
// ****************************************************************************

class QUERY_API avtNodeCoordsQuery : public avtDatasetQuery
{
  public:
                             avtNodeCoordsQuery();
    virtual                 ~avtNodeCoordsQuery(); 

    virtual const char      *GetType(void) { return "avtNodeCoordsQuery"; };
    virtual const char      *GetDescription(void) 
                                  { return "Getting node coords."; };

    virtual void             SetInputParams(const MapNode &);
    static  void             GetDefaultInputParams(MapNode &);

    virtual void             PerformQuery(QueryAttributes *);
    virtual bool             OriginalData(void) { return true; };

  protected:

    virtual void             Execute(vtkDataSet*, const int){;};
    bool                     FindLocalCoord(double[3]);
    bool                     FindGlobalCoord(double[3]);
  
  private:
    int   domain;
    int   element;
    bool  useGlobalId;
};

#endif

