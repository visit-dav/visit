// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.


// ************************************************************************* //
//                         vtkPointGlyphMapper.h                             //
// ************************************************************************* //


#ifndef vtkPointGlyphMapper_h
#define vtkPointGlyphMapper_h

#include <vtkPolyDataMapper.h>
#include <vtkNew.h>

#include <plotter_exports.h>

#include <string>
#include <GlyphTypes.h>

class vtkLookupTable;
class vtkPointGlyphMapperHelper;
class vtkPointMapper;
class vtkTrivialProducer;
class vtkVisItGlyph3D;
class vtkVisItPolyDataNormals;

// ****************************************************************************
// Class: vtkPointGlyphMapper
//
// Purpose:
//   Subclass of vtkPolyDataMapper that can draw points as glyphs, points,
//   or sphere imposters.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 17, 2016
//
//  Modifications:
//
// ****************************************************************************

class PLOTTER_API vtkPointGlyphMapper : public vtkPolyDataMapper
{
public:
  static vtkPointGlyphMapper *New();
  vtkTypeMacro(vtkPointGlyphMapper,vtkPolyDataMapper);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void Render(vtkRenderer *, vtkActor *) override;
  void ReleaseGraphicsResources(vtkWindow *) override;

  // To pass on to underlying mappers
  void SetScalarRange(double, double) override;
  void SetLookupTable(vtkLookupTable *lut);

  vtkGetMacro(SpatialDimension, int);
  vtkSetMacro(SpatialDimension, int);

  void SetGlyphType(GlyphType);
  GlyphType  GetGlyphType(void);
  void SetScale(double);
  void DataScalingOff(void);
  void DataScalingOn(const std::string &, int);
  void ColorByScalarOn(const std::string &);
  void ColorByScalarOff(void);
  bool SetFullFrameScaling(bool useScale, const double *s);

protected:
  vtkPointGlyphMapper();
  ~vtkPointGlyphMapper();

  void RenderPiece(vtkRenderer *, vtkActor *) override;

  // Description:
  bool        UseImposters;
  bool        PointDataInitialized;
  int         SpatialDimension;


  // Cached variables
  vtkNew<vtkPolyData> PointPolyData;
  vtkNew<vtkTrivialProducer> PointOutput;
  vtkNew<vtkVisItGlyph3D>         GlyphFilter;
  vtkNew<vtkVisItPolyDataNormals> NormalsFilter;
  vtkNew<vtkPolyDataMapper>       GlyphMapper;
  vtkNew<vtkPointMapper>          PointMapper;

  void UpdatePointData();

private:
  vtkPointGlyphMapper(const vtkPointGlyphMapper&);
  void operator=(const vtkPointGlyphMapper&);

  vtkPointGlyphMapperHelper *Helper;
};

#endif
