// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef __vtkVisItDataSetMapper_h
#define __vtkVisItDataSetMapper_h
#include <plotter_exports.h>
#include <vtkDataSetMapper.h>

class vtkLookupTable;

//
//  Subclass of vtkDataSetMapper, handles LUMINANCE and VALUE
//  renderinge modes (used by VisIt's Cinema support) by setting up
//  either allwhite or grayscale vtkLookupTable. All other rendering
//  is handled by Superclass.
//
//  KSB 04-13-2021
//  Note: This should be do-able via vtk's rendering passes, but couldn't
//  figure out the correct place in VisIt's pipeline for their use.
//  This route was part of Brad's original Cinema support, so it was
//  a quick way to re-enable the Cinema composite functionality.
//
//  KSB 04-15-2021
//  Note: in VisWinRendering.C, OSPRAY creates its own override of
//  vtkDataSetMapper.  The OSPRAY override was modified to override this
//  class instead.  If this class is ever removed, the OSPRAY override will
//  need to be changed back to vtkDataSetMapper.

class PLOTTER_API vtkVisItDataSetMapper : public vtkDataSetMapper
{
public:
  static vtkVisItDataSetMapper *New();
  vtkTypeMacro(vtkVisItDataSetMapper, vtkDataSetMapper);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  void Render(vtkRenderer *ren, vtkActor *act) override;

   // Let's have a way of forcing the mapper to render different ways with
   // an easy global switch.
   typedef enum
   {
       RENDERING_MODE_NORMAL,
       RENDERING_MODE_LUMINANCE,
       RENDERING_MODE_VALUE
   } RenderingMode;

   static void SetRenderingMode(RenderingMode);
   static RenderingMode GetRenderingMode();
protected:
   //RenderingMode rMode;
   vtkLookupTable *allwhite;
   vtkLookupTable *grayscale;

  vtkVisItDataSetMapper();
  ~vtkVisItDataSetMapper() override;

private:
  vtkVisItDataSetMapper(const vtkVisItDataSetMapper&) = delete;
  void operator=(const vtkVisItDataSetMapper&) = delete;
};

#endif
