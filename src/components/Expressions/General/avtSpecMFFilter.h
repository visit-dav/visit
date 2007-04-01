// ************************************************************************* //
//                             avtSpecMFFilter.h                             //
// ************************************************************************* //

#ifndef AVT_SPECMF_FILTER_H
#define AVT_SPECMF_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ConstExpr;

// ****************************************************************************
//  Class: avtSpecMFFilter
//
//  Purpose:
//      Creates the species mass fraction at each cell.
//          
//  Programmer: Jeremy Meredith
//  Creation:   June  8, 2004
//
//  Modifications:
//
// ****************************************************************************

class EXPRESSION_API avtSpecMFFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtSpecMFFilter();
    virtual                  ~avtSpecMFFilter();

    virtual const char       *GetType(void) { return "avtSpecMFFilter"; };
    virtual const char       *GetDescription(void)
                                     {return "Calculating Species Mass Frac";};
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);

  protected:
    bool                      issuedWarning;

    virtual avtPipelineSpecification_p
                              PerformRestriction(avtPipelineSpecification_p);

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void)  { return false; };
    virtual void              PreExecute(void);

    void                      AddMaterial(ConstExpr *);
    void                      AddSpecies(ConstExpr *);

    bool                      weightByVF;
    std::vector<std::string>  matNames;
    std::vector<int>          matIndices;
    std::vector<std::string>  specNames;
    std::vector<int>          specIndices;
};


#endif


