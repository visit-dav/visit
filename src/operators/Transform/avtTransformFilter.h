// ************************************************************************* //
//  File: avtTransformFilter.h
// ************************************************************************* //

#ifndef AVT_Transform_FILTER_H
#define AVT_Transform_FILTER_H


#include <avtPluginStreamer.h>
#include <avtTransform.h>
#include <TransformAttributes.h>
#include <vtkMatrix4x4.h>

class vtkDataSet;


// ****************************************************************************
//  Class: avtTransformFilter
//
//  Purpose:
//      A plugin operator for Transform.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 24, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Nov 28 16:59:53 PST 2001
//    Added RefashionDataObjectInfo, PerformRestriction.
//
//    Hank Childs, Tue Mar  5 16:05:25 PST 2002
//    Removed RefashionDataObjectInfo, since what is was doing is also
//    applicable to the base class, so I pushed the logic into it.
//
//    Kathleen Bonnell, Thu Apr 10 11:07:48 PDT 2003 
//    Store inverse matrix for possible use later in the pipeline. 
//    Added PostExecute method.
//    
// ****************************************************************************

class avtTransformFilter : public virtual avtTransform,
                           public virtual avtPluginStreamer
{
  public:
                         avtTransformFilter();
    virtual             ~avtTransformFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtTransformFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Transforming"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    TransformAttributes   atts;
    vtkMatrix4x4         *M;
    vtkMatrix4x4         *invM;

    void                  SetupMatrix();
    virtual vtkMatrix4x4 *GetTransform() { SetupMatrix(); return M; };
    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);
    virtual void          RefashionDataObjectInfo(void);
    virtual void          PostExecute(void);
};


#endif
