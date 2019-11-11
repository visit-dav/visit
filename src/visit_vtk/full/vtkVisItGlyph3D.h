/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItGlyph3D.h,v $
  Language:  C++
  Date:      $Date: 2003/03/25 14:58:57 $
  Version:   $Revision: 1.57 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItGlyph3D - copy oriented and scaled glyph geometry to every input point
// .SECTION Description
// vtkVisItGlyph3D is a filter that copies a geometric representation (called
// a glyph) to every point in the input dataset. The glyph is defined with
// polygonal data from a source filter input. The glyph may be oriented
// along the input vectors or normals, and it may be scaled according to
// scalar data or vector magnitude. More than one glyph may be used by
// creating a table of source objects, each defining a different glyph. If a
// table of glyphs is defined, then the table can be indexed into by using
// either scalar value or vector magnitude.
//
// To use this object you'll have to provide an input dataset and a source
// to define the glyph. Then decide whether you want to scale the glyph and
// how to scale the glyph (using scalar value or vector magnitude). Next
// decide whether you want to orient the glyph, and whether to use the
// vector data or normal data to orient it. Finally, decide whether to use a
// table of glyphs, or just a single glyph. If you use a table of glyphs,
// you'll have to decide whether to index into it with scalar value or with
// vector magnitude.
//
// .SECTION Caveats
// The scaling of the glyphs is controlled by the ScaleFactor ivar multiplied
// by the scalar value at each point (if VTK_SCALE_BY_SCALAR is set), or
// multiplied by the vector magnitude (if VTK_SCALE_BY_VECTOR is set),
// Alternatively (if VTK_SCALE_BY_VECTORCOMPONENTS is set), the scaling
// may be specified for x,y,z using the vector components. The
// scale factor can be further controlled by enabling clamping using the
// Clamping ivar. If clamping is enabled, the scale is normalized by the
// Range ivar, and then multiplied by the scale factor. The normalization
// process includes clamping the scale value between (0,1).
//
// Typically this object operates on input data with scalar and/or vector
// data. However, scalar and/or vector aren't necessary, and it can be used
// to copy data from a single source to each point. In this case the scale
// factor can be used to uniformly scale the glyphs.
//
// The object uses "vector" data to scale glyphs, orient glyphs, and/or index
// into a table of glyphs. You can choose to use either the vector or normal
// data at each input point. Use the method SetVectorModeToUseVector() to use
// the vector input data, and SetVectorModeToUseNormal() to use the
// normal input data.
//
// If you do use a table of glyphs, make sure to set the Range ivar to make
// sure the index into the glyph table is computed correctly.
//
// You can turn off scaling of the glyphs completely by using the Scaling
// ivar. You can also turn off scaling due to data (either vector or scalar)
// by using the SetScaleModeToDataScalingOff() method.

// .SECTION See Also
// vtkTensorGlyph

#ifndef __vtkVisItGlyph3D_h
#define __vtkVisItGlyph3D_h
#include <visit_vtk_exports.h>

#include <vtkPolyDataAlgorithm.h>

#define VTK_SCALE_BY_SCALAR 0
#define VTK_SCALE_BY_VECTOR 1
#define VTK_SCALE_BY_TENSOR 2
#define VTK_SCALE_BY_VECTORCOMPONENTS 3
#define VTK_DATA_SCALING_OFF 4

#define VTK_COLOR_BY_SCALE  0
#define VTK_COLOR_BY_SCALAR 1
#define VTK_COLOR_BY_VECTOR 2
#define VTK_COLOR_BY_SINGLE_COLOR 3

#define VTK_USE_VECTOR 0
#define VTK_USE_NORMAL 1
#define VTK_VECTOR_ROTATION_OFF 2

#define VTK_INDEXING_OFF 0
#define VTK_INDEXING_BY_SCALAR 1
#define VTK_INDEXING_BY_VECTOR 2

// ***************************************************************************
//  Class: vtkVisItGlyph3D
//
//  Modifications:
//    Eric Brugger, Thu Jan 10 13:00:03 PST 2013
//    Modified to inherit from vtkPolyDataAlgorithm.
//
// ***************************************************************************

class VISIT_VTK_API vtkVisItGlyph3D : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkVisItGlyph3D,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description
  // Construct object with scaling on, scaling mode is by scalar value,
  // scale factor = 1.0, the range is (0,1), orient geometry is on, and
  // orientation is by vector. Clamping and indexing are turned off. No
  // initial sources are defined.
  static vtkVisItGlyph3D *New();

  // Description:
  // Set the source to use for the glyph. Old style. See SetSourceConnection.
  void SetSourceData(vtkPolyData *pd) {this->SetSourceData(0,pd);}

  // Description:
  // Specify a source object at a specified table location.
  // Old style. See SetSourceConnection.
  void SetSourceData(int id, vtkPolyData *pd);

  // Description:
  // Specify a source object at a specified table location. New style.
  // Source connection is stored in port 1. This method is equivalent
  // to SetInputConnection(1, id, outputPort).
  void SetSourceConnection(int id, vtkAlgorithmOutput* algOutput);
  void SetSourceConnection(vtkAlgorithmOutput* algOutput)
    {
      this->SetSourceConnection(0, algOutput);
    }

  // Description:
  // Get a pointer to a source object at a specified table location.
  vtkPolyData *GetSource(int id=0);

  // Description:
  // Turn on/off scaling of source geometry.
  vtkSetMacro(Scaling,bool);
  vtkGetMacro(Scaling,bool);
  vtkBooleanMacro(Scaling,bool);

  // Description:
  // Either scale by scalar or by vector/normal magnitude.
  vtkSetMacro(ScaleMode,int);
  vtkGetMacro(ScaleMode,int);
  void SetScaleModeToScaleByScalar()
    {this->SetScaleMode(VTK_SCALE_BY_SCALAR);}
  void SetScaleModeToScaleByVector()
    {this->SetScaleMode(VTK_SCALE_BY_VECTOR);}
  void SetScaleModeToScaleByTensor()
    {this->SetScaleMode(VTK_SCALE_BY_TENSOR);}
  void SetScaleModeToScaleByVectorComponents()
    {this->SetScaleMode(VTK_SCALE_BY_VECTORCOMPONENTS);}
  void SetScaleModeToDataScalingOff()
    {this->SetScaleMode(VTK_DATA_SCALING_OFF);}
  const char *GetScaleModeAsString();

  // Description:
  // Either color by scale, scalar or by vector/normal magnitude.
  vtkSetMacro(ColorMode,int);
  vtkGetMacro(ColorMode,int);
  void SetColorModeToColorByScale()
    {this->SetColorMode(VTK_COLOR_BY_SCALE);}
  void SetColorModeToColorByScalar()
    {this->SetColorMode(VTK_COLOR_BY_SCALAR);}
  void SetColorModeToColorByVector()
    {this->SetColorMode(VTK_COLOR_BY_VECTOR);}
  void SetColorModeToColorBySingleColor()
    {this->SetColorMode(VTK_COLOR_BY_SINGLE_COLOR);}
  const char *GetColorModeAsString();

  // Description:
  // Specify scale factor to scale object by.
  vtkSetMacro(ScaleFactor,double);
  vtkGetMacro(ScaleFactor,double);

  // Description:
  // Specify range to map scalar values into.
  vtkSetVector2Macro(Range,double);
  vtkGetVectorMacro(Range,double,2);

  // Description:
  // Turn on/off orienting of input geometry along vector/normal.
  vtkSetMacro(Orient,bool);
  vtkGetMacro(Orient,bool);
  vtkBooleanMacro(Orient,bool);

  // Description:
  // Turn on/off clamping of "scalar" values to range. (Scalar value may be
  //  vector magnitude if ScaleByVector() is enabled.)
  vtkSetMacro(Clamping,bool);
  vtkGetMacro(Clamping,bool);
  vtkBooleanMacro(Clamping,bool);

  // Description:
  // Specify whether to use vector or normal to perform vector operations.
  vtkSetMacro(VectorMode,int);
  vtkGetMacro(VectorMode,int);
  void SetVectorModeToUseVector() {this->SetVectorMode(VTK_USE_VECTOR);}
  void SetVectorModeToUseNormal() {this->SetVectorMode(VTK_USE_NORMAL);}
  void SetVectorModeToVectorRotationOff()
    {this->SetVectorMode(VTK_VECTOR_ROTATION_OFF);}
  const char *GetVectorModeAsString();

  // Description:
  // Index into table of sources by scalar, by vector/normal magnitude, or
  // no indexing. If indexing is turned off, then the first source glyph in
  // the table of glyphs is used.
  vtkSetMacro(IndexMode,int);
  vtkGetMacro(IndexMode,int);
  void SetIndexModeToScalar() {this->SetIndexMode(VTK_INDEXING_BY_SCALAR);}
  void SetIndexModeToVector() {this->SetIndexMode(VTK_INDEXING_BY_VECTOR);}
  void SetIndexModeToOff() {this->SetIndexMode(VTK_INDEXING_OFF);}
  const char *GetIndexModeAsString();

  // Description:
  // Enable/disable the generation of point ids as part of the output. The
  // point ids are the id of the input generating point. The point ids are
  // stored in the output point field data and named "InputPointIds". Point
  // generation is useful for debugging and pick operations.
  vtkSetMacro(GeneratePointIds,bool);
  vtkGetMacro(GeneratePointIds,bool);
  vtkBooleanMacro(GeneratePointIds,bool);

  // Description:
  // Enable/disable the 2D treatment of vectors
  vtkSetMacro(TreatVectorsAs2D,bool);
  vtkGetMacro(TreatVectorsAs2D,bool);
  vtkBooleanMacro(TreatVectorsAs2D,bool);

  // Description:
  // Set/Get the name of the PointIds array if generated. By default the Ids
  // are named "InputPointIds", but this can be changed with this function.
  vtkSetStringMacro(PointIdsName);
  vtkGetStringMacro(PointIdsName);

  // Description:
  // If you want to use an arbitrary scalars array, then set its name here.
  // By default this in NULL and the filter will use the active scalar array.
  vtkGetStringMacro(InputScalarsSelection);
  void SelectInputScalars(const char *fieldName)
    {this->SetInputScalarsSelection(fieldName);}

  // Description:
  // If you want to use an arbitrary vectors array, then set its name here.
  // By default this in NULL and the filter will use the active vector array.
  vtkGetStringMacro(InputVectorsSelection);
  void SelectInputVectors(const char *fieldName)
    {this->SetInputVectorsSelection(fieldName);}

  // Description:
  // If you want to use an arbitrary normals array, then set its name here.
  // By default this in NULL and the filter will use the active normal array.
  vtkGetStringMacro(InputNormalsSelection);
  void SelectInputNormals(const char *fieldName)
    {this->SetInputNormalsSelection(fieldName);}

  vtkGetStringMacro(ScalarsForColoring);
  void SelectScalarsForColoring(const char *fieldName)
    {this->SetScalarsForColoring(fieldName);}

  vtkGetStringMacro(ScalarsForScaling);
  void SelectScalarsForScaling(const char *fieldName)
    {this->SetScalarsForScaling(fieldName);}

  vtkGetStringMacro(VectorsForColoring);
  void SelectVectorsForColoring(const char *fieldName)
    {this->SetVectorsForColoring(fieldName);}

  vtkGetStringMacro(VectorsForScaling);
  void SelectVectorsForScaling(const char *fieldName)
    {this->SetVectorsForScaling(fieldName);}

  vtkGetStringMacro(TensorsForScaling);
  void SelectTensorsForScaling(const char *fieldName)
    {this->SetTensorsForScaling(fieldName);}

  int SetFullFrameScaling(int useIt, const double *s);

protected:
  vtkVisItGlyph3D();
  ~vtkVisItGlyph3D();

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;
  virtual int RequestUpdateExtent(vtkInformation *,
                                  vtkInformationVector **,
                                  vtkInformationVector *) override;
  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

  vtkPolyData* GetSource(int idx, vtkInformationVector *sourceInfo);

  vtkPolyData **Source; // Geometry to copy to each point
  bool Scaling; // Determine whether scaling of geometry is performed
  int ScaleMode; // Scale by scalar value or vector magnitude
  int ColorMode; // new scalars based on scale, scalar or vector
  double ScaleFactor; // Scale factor to use to scale geometry
  double Range[2]; // Range to use to perform scalar scaling
  bool Orient; // boolean controls whether to "orient" data
  int VectorMode; // Orient/scale via normal or via vector data
  bool Clamping; // whether to clamp scale factor
  int IndexMode; // what to use to index into glyph table
  bool GeneratePointIds; // produce input points ids for each output point
  bool TreatVectorsAs2D; // glyph only the 2D portions of vectors
  char *PointIdsName;

  char *InputScalarsSelection;
  char *InputVectorsSelection;
  char *InputNormalsSelection;
  vtkSetStringMacro(InputScalarsSelection);
  vtkSetStringMacro(InputVectorsSelection);
  vtkSetStringMacro(InputNormalsSelection);

  char *ScalarsForColoring;
  char *ScalarsForScaling;
  char *VectorsForColoring;
  char *VectorsForScaling;
  char *TensorsForScaling;

  vtkSetStringMacro(ScalarsForColoring);
  vtkSetStringMacro(ScalarsForScaling);
  vtkSetStringMacro(VectorsForColoring);
  vtkSetStringMacro(VectorsForScaling);
  vtkSetStringMacro(TensorsForScaling);

  int    UseFullFrameScaling;
  double FullFrameScaling[3];

private:
  vtkVisItGlyph3D(const vtkVisItGlyph3D&);  // Not implemented.
  void operator=(const vtkVisItGlyph3D&);  // Not implemented.
};

// Description:
// Return the method of scaling as a descriptive character string.
inline const char *vtkVisItGlyph3D::GetScaleModeAsString(void)
{
  if ( this->ScaleMode == VTK_SCALE_BY_SCALAR )
  {
    return "ScaleByScalar";
  }
  else if ( this->ScaleMode == VTK_SCALE_BY_VECTOR )
  {
    return "ScaleByVector";
  }
  else if ( this->ScaleMode == VTK_SCALE_BY_TENSOR )
  {
    return "ScaleByTensor";
  }
  else
  {
    return "DataScalingOff";
  }
}

// Description:
// Return the method of coloring as a descriptive character string.
inline const char *vtkVisItGlyph3D::GetColorModeAsString(void)
{
  if ( this->ColorMode == VTK_COLOR_BY_SCALAR )
  {
    return "ColorByScalar";
  }
  else if ( this->ColorMode == VTK_COLOR_BY_VECTOR )
  {
    return "ColorByVector";
  }
  else if ( this->ColorMode == VTK_COLOR_BY_SINGLE_COLOR )
  {
    return "ColorBySingleColor";
  }
  else
  {
    return "ColorByScale";
  }
}

// Description:
// Return the vector mode as a character string.
inline const char *vtkVisItGlyph3D::GetVectorModeAsString(void)
{
  if ( this->VectorMode == VTK_USE_VECTOR)
  {
    return "UseVector";
  }
  else if ( this->VectorMode == VTK_USE_NORMAL)
  {
    return "UseNormal";
  }
  else
  {
    return "VectorRotationOff";
  }
}

// Description:
// Return the index mode as a character string.
inline const char *vtkVisItGlyph3D::GetIndexModeAsString(void)
{
  if ( this->IndexMode == VTK_INDEXING_OFF)
  {
    return "IndexingOff";
  }
  else if ( this->IndexMode == VTK_INDEXING_BY_SCALAR)
  {
    return "IndexingByScalar";
  }
  else
  {
    return "IndexingByVector";
  }
}

#endif
