// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.


#ifndef __vtkVisItTensorGlyph_h
#define __vtkVisItTensorGlyph_h

#include <visit_vtk_exports.h>

#include <vtkTensorGlyph.h>

// ****************************************************************************
//  Class: vtkVisItTensorGlyph
//
//  Purpose:
//    Extension of vtkTensorGlyph, to ensure avtOriginaCellNumbers and
//    avtOriginalNodeNumbers are preserved in the output.
//    Also fix a pointdata scalars scaling issue when ColorMode is
//    COLOR_BY_EIGENVALUES, ScaleFactor > 0 and Glyphs are being colored.
//
// ****************************************************************************

class VISIT_VTK_API vtkVisItTensorGlyph : public vtkTensorGlyph
{
public:
  vtkTypeMacro(vtkVisItTensorGlyph, vtkTensorGlyph)
  static vtkVisItTensorGlyph *New();

protected:
  vtkVisItTensorGlyph();
  ~vtkVisItTensorGlyph() override;

  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *) override;

private:
  vtkVisItTensorGlyph(const vtkVisItTensorGlyph&) = delete;
  void operator=(const vtkVisItTensorGlyph&) = delete;
};

#endif
