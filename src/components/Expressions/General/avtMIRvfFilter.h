// ************************************************************************* //
//                              avtMIRvfFilter.h                             //
// ************************************************************************* //

#ifndef AVT_MIRVF_FILTER_H
#define AVT_MIRVF_FILTER_H

#include <avtSingleInputExpressionFilter.h>

class     vtkDataArray;
class     ArgsExpr;
class     ExprPipelineState;
class     ConstExpr;


// ****************************************************************************
//  Class: avtMIRvfFilter
//
//  Purpose:
//      Calculates the volume fraction as computed by the MIR.
//          
//  Programmer: Hank Childs
//  Creation:   December 31, 2004
//
// ****************************************************************************

class EXPRESSION_API avtMIRvfFilter : public avtSingleInputExpressionFilter
{
  public:
                              avtMIRvfFilter();
    virtual                  ~avtMIRvfFilter();

    virtual const char       *GetType(void) { return "avtMIRvfFilter"; };
    virtual const char       *GetDescription(void)
                                            {return "Calculating MIR VF";};
    virtual void              ProcessArguments(ArgsExpr*, ExprPipelineState *);
    virtual int               NumVariableArguments(void) { return 3; };

  protected:
    bool                      issuedWarning;
    std::vector<std::string>  matNames;
    std::vector<int>          matIndices;
    std::string               zoneid_name;
    std::string               volume_name;

    virtual avtPipelineSpecification_p
                              PerformRestriction(avtPipelineSpecification_p);

    virtual vtkDataArray     *DeriveVariable(vtkDataSet *);
    virtual bool              IsPointVariable(void)  { return false; };
    virtual void              PreExecute(void);
    virtual void              RefashionDataObjectInfo(void);

    void                      AddMaterial(ConstExpr *);
    void                      GetMaterialList(std::vector<bool> &);
};


#endif


