// ************************************************************************* //
//                            avtCustomTransform.h                           //
// ************************************************************************* //

#ifndef AVT_CUSTOM_TRANSFORM_H
#define AVT_CUSTOM_TRANSFORM_H

#include <filters_exports.h>

#include <avtTransform.h>

// ****************************************************************************
//  Class: avtCustomTransform
//
//  Purpose:
//      Does any 4x4 matrix transformation.  Matrix is specified through a
//      method.
//
//  Programmer: Hank Childs
//  Creation:   January 21, 2006
//
// ****************************************************************************

class AVTFILTERS_API avtCustomTransform : public virtual avtTransform
{
  public:
                                avtCustomTransform();
    virtual                    ~avtCustomTransform();

    virtual const char         *GetType(void) { return "avtCustomTransform"; };
    void                        SetMatrix(vtkMatrix4x4 *);

  protected:
    vtkMatrix4x4               *mat;
    virtual vtkMatrix4x4       *GetTransform() { return mat; };
};


#endif


