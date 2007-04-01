// ************************************************************************* //
//                             avtDataIdFilter.h                             //
// ************************************************************************* //

#ifndef AVT_ZONE_ID_FILTER_H
#define AVT_ZONE_ID_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;


// ****************************************************************************
//  Class: avtDataIdFilter
//
//  Purpose:
//      Identifies the data id for each datum in the problem.  Example data
//      include "zones" or "nodes".
//          
//  Programmer: Hank Childs
//  Creation:   September 23, 2004
//
// ****************************************************************************

class EXPRESSION_API avtDataIdFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtDataIdFilter();
    virtual                  ~avtDataIdFilter();

    virtual const char       *GetType(void) { return "avtDataIdFilter"; };
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

  protected:
    bool                      doZoneIds;
    bool                      doGlobalNumbering;
    bool                      haveIssuedWarning;

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void)  { return (!doZoneIds); };

    virtual avtPipelineSpecification_p
                              PerformRestriction(avtPipelineSpecification_p);
};


#endif


