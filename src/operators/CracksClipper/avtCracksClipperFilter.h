// ************************************************************************* //
//  File: avtCracksClipperFilter.h
// ************************************************************************* //

#ifndef AVT_CracksClipper_FILTER_H
#define AVT_CracksClipper_FILTER_H


#include <avtPluginStreamer.h>
#include <CracksClipperAttributes.h>


class vtkDataSet;


// ****************************************************************************
//  Class: avtCracksClipperFilter
//
//  Purpose:
//    A plugin operator for clipping away Cracks.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Thu Oct 13 08:17:36 PDT 2005
//
// ****************************************************************************

class avtCracksClipperFilter : public avtPluginStreamer
{
  public:
                         avtCracksClipperFilter();
    virtual             ~avtCracksClipperFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtCracksClipperFilter"; };
    virtual const char  *GetDescription(void)
                             { return "CracksClipper"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    CracksClipperAttributes   atts;

    virtual vtkDataSet   *ExecuteData(vtkDataSet *, int, std::string);
    virtual void          PostExecute(void);
    virtual void          RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);

  private:
    bool                  NeedsProcessing(vtkDataSet *, bool *np);
    vtkDataSet           *RemoveCracks(vtkDataSet *inds, int whichCrack);
    void                  RemoveExtraArrays(vtkDataSet *ds, bool v = false);
};


#endif
