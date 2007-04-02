// ************************************************************************* //
//                             avtMoleculeFilter.h                           //
// ************************************************************************* //

#ifndef AVT_MOLECULE_FILTER_H
#define AVT_MOLECULE_FILTER_H


#include <MoleculeAttributes.h>
#include <avtStreamer.h>
#include <set>
#include <vector>
#include <string>

class  vtkMoleculeReduceFilter;
class  vtkVertexFilter;


// ****************************************************************************
//  Class: avtMoleculeFilter
//
//  Purpose:
//      Do some work to make the molecule plot happy.
//
//  Programmer: Jeremy Meredith
//  Creation:   February 14, 2006
//
//  Modifications:
//
// ****************************************************************************

class avtMoleculeFilter : public avtStreamer
{
  public:
                              avtMoleculeFilter();
    virtual                  ~avtMoleculeFilter();

    virtual const char       *GetType(void)   { return "avtMoleculeFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Creating molecules"; };

    virtual void              ReleaseData(void);
    void                      SetAtts(const MoleculeAttributes*);

  protected:
    MoleculeAttributes        atts;

    std::string               name;
    std::set<int>             used_values;


    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
    virtual vtkDataSet       *ExecuteData(vtkDataSet *, int, std::string);
    virtual void              RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p
                              PerformRestriction(avtPipelineSpecification_p);
};


#endif


