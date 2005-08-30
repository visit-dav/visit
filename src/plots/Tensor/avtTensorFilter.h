// ************************************************************************* //
//                             avtTensorFilter.h                             //
// ************************************************************************* //

#ifndef AVT_TENSOR_FILTER_H
#define AVT_TENSOR_FILTER_H


#include <avtStreamer.h>

class  vtkTensorReduceFilter;
class  vtkVertexFilter;


// ****************************************************************************
//  Class: avtTensorFilter
//
//  Purpose:
//      A filter that takes in tensor data and creates tensor glyphs as poly
//      data.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//  Modifications:
//    Kathleen Bonnell, Tue Aug 30 15:11:01 PDT 2005
//    Added keepNodeZone & PerformRestriction.
//
// ****************************************************************************

class avtTensorFilter : public avtStreamer
{
  public:
                              avtTensorFilter(bool, int);
    virtual                  ~avtTensorFilter();

    virtual const char       *GetType(void)   { return "avtTensorFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Creating tensors"; };

    virtual void              ReleaseData(void);

    bool                      Equivalent(bool, int);

    void                      SetStride(int);
    void                      SetNTensors(int);

  protected:
    bool                      keepNodeZone;
    bool                      useStride;
    int                       stride;
    int                       nTensors;

    vtkVertexFilter          *vertex;
    vtkTensorReduceFilter    *reduce;

    virtual vtkDataSet       *ExecuteData(vtkDataSet *, int, std::string);
    virtual void              RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p
                              PerformRestriction(avtPipelineSpecification_p);
};


#endif


