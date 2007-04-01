// ************************************************************************* //
//                              avtTransform.h                               //
// ************************************************************************* //

#ifndef AVT_TRANSFORM_H
#define AVT_TRANSFORM_H

#include <pipeline_exports.h>

#include <avtStreamer.h>

class     vtkMatrix4x4;
class     vtkRectilinearGrid;


// ****************************************************************************
//  Class: avtTransform
//
//  Purpose:
//      Does any 4x4 matrix transformation.  Derived types specify the matrix.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001
//    Changed ExecuteDomain to ExecuteData.
//
//    Hank Childs, Wed Jun  6 09:12:58 PDT 2001
//    Removed CalcDomainList.
//
//    Jeremy Meredith, Mon Sep 24 14:21:13 PDT 2001
//    Added RefashionDataObjectInfo.  Made inheritance virtual.
//
//    Hank Childs, Mon Jan 14 09:01:18 PST 2002
//    Added data member for transform filter to fix memory leak.
//
//    Hank Childs, Mon Aug  5 10:29:22 PDT 2002
//    Added individual routines to transform rectilinear grids based on what
//    type of transformation is done.
//
//    Hank Childs, Tue Sep 10 15:21:28 PDT 2002
//    Perform all memory management at the domain (ExecuteData) level.
//
// ****************************************************************************

class PIPELINE_API avtTransform : public virtual avtStreamer
{
  public:
                                avtTransform() {;};
    virtual                    ~avtTransform() {;};

    virtual const char         *GetType(void) { return "avtTransform"; };
    virtual const char         *GetDescription(void)
                                        { return "Transforming data"; };

  protected:
    virtual vtkDataSet         *ExecuteData(vtkDataSet *, int, std::string);
    virtual vtkMatrix4x4       *GetTransform() = 0;
    virtual void                RefashionDataObjectInfo(void);

    vtkDataSet                 *TransformRectilinear(vtkRectilinearGrid *);
    bool                        OutputIsRectilinear(vtkMatrix4x4 *);
    vtkDataSet                 *TransformRectilinearToRectilinear(
                                                         vtkRectilinearGrid *);
    vtkDataSet                 *TransformRectilinearToCurvilinear(
                                                         vtkRectilinearGrid *);
};


#endif


