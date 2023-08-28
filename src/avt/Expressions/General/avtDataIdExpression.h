// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtDataIdExpression.h                             //
// ************************************************************************* //

#ifndef AVT_ZONE_ID_FILTER_H
#define AVT_ZONE_ID_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtDataIdExpression
//
//  Purpose:
//      Identifies the data id for each datum in the problem.  Example data
//      include "zones" or "nodes".
//          
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
//  Modifications:
//    Brad Whitlock, Tue Sep 10 16:12:14 PDT 2013
//    Added ability to create IJK indices.
//
//    Chris Laganella, Fri Feb  4 19:29:10 EST 2022
//    Added ability to create domain ids.
// ****************************************************************************

class EXPRESSION_API avtDataIdExpression : public avtSingleInputExpressionFilter
{
  public:
                              avtDataIdExpression();
    virtual                  ~avtDataIdExpression();

    virtual const char       *GetType(void) { return "avtDataIdExpression"; };
    virtual const char       *GetDescription(void)
                                           {return "Assigning IDs.";};
    virtual void              PreExecute(void);

    void                      CreateZoneIds(void)
                                        { doZoneIds = true; };
    void                      CreateNodeIds(void)
                                        { doZoneIds = false; };
    void                      CreateGlobalNumbering(void)
                                        { doGlobalNumbering = true; };
    void                      CreateLocalNumbering(void)
                                        { doGlobalNumbering = false; };
    void                      CreateIJK(void) { doIJK = true; }
    void                      CreateDomainIds(void) { doDomainIds = true; }

  protected:
    bool                      doZoneIds;
    bool                      doGlobalNumbering;
    bool                      doIJK;
    bool                      doDomainIds;
    bool                      haveIssuedWarning;

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *, int currentDomainsIndex);
    virtual bool              IsPointVariable(void)  { return (!doZoneIds); };

    virtual int               GetVariableDimension();
    virtual avtContract_p
                              ModifyContract(avtContract_p);
};


#endif


