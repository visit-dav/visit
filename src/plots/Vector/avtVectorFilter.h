// ************************************************************************* //
//                             avtVectorFilter.h                             //
// ************************************************************************* //

#ifndef AVT_VECTOR_FILTER_H
#define AVT_VECTOR_FILTER_H


#include <avtStreamer.h>

class  vtkVectorReduceFilter;
class  vtkVertexFilter;


// ****************************************************************************
//  Class: avtVectorFilter
//
//  Purpose:
//      A filter that takes in vector data and creates vector glyphs as poly
//      data.
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr 10 11:51:18 PDT 2001
//    Renamed ExecuteDomain as ExecuteData.
//
//    Hank Childs, Thu Aug 30 17:30:48 PDT 2001
//    Added the vertex filter.
//
// ****************************************************************************

class avtVectorFilter : public avtStreamer
{
  public:
                              avtVectorFilter(bool, int);
    virtual                  ~avtVectorFilter();

    virtual const char       *GetType(void)   { return "avtVectorFilter"; };
    virtual const char       *GetDescription(void)
                                  { return "Creating vectors"; };

    virtual void              ReleaseData(void);

    bool                      Equivalent(bool, int);

    void                      SetStride(int);
    void                      SetNVectors(int);

  protected:
    bool                      useStride;
    int                       stride;
    int                       nVectors;

    vtkVertexFilter          *vertex;
    vtkVectorReduceFilter    *reduce;

    virtual vtkDataSet       *ExecuteData(vtkDataSet *, int, std::string);
    virtual void              RefashionDataObjectInfo(void);
};


#endif


