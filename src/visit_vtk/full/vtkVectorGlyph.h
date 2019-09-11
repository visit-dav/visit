// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkVectorGlyph -- Creates the appropriate glyph for a vector.
//
// .SECTION Description
// Creates the poly data to be used by a glyph filter for vector data.  This
// is a line with a head.  The head is not capped.  The size of the head may
// be set and the head may be turned off completely.
//

#ifndef __vtkVectorGlyph_h
#define __vtkVectorGlyph_h
#include <visit_vtk_exports.h>

#include <vtkPolyDataAlgorithm.h>

//  Modifications:
//    Jeremy Meredith, Fri Nov 21 11:25:27 PST 2003
//    Added offset to allow glyphs to originate/terminate at the point.
//
//    Jeremy Meredith, Mon Mar 19 14:33:15 EDT 2007
//    Added "HighQuality", which ups the number of facets, "LineStem"
//    which determines if the stem is a line or a cylinder(3d)/rectangle(2d),
//    "CapEnds" which caps the cone's base (and the cylinder's if
//    we're in 3D, i.e. if ConeHead is true), and the "StemWidth" which
//    defines the width of the stem if we're drawing is as a cyl/rect.
//
//    Dave Pugmire, Mon Jul 19 09:38:17 EDT 2010
//    Add ellipsoid glyphing.        

class VISIT_VTK_API vtkVectorGlyph : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkVectorGlyph, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Instantiate a stride filter that throws away nine of every ten elements.
  static vtkVectorGlyph *New();
  vtkSetMacro(HighQuality,bool);
  vtkGetMacro(HighQuality,bool);
  vtkBooleanMacro(HighQuality,bool);

  vtkSetMacro(CapEnds,bool);
  vtkGetMacro(CapEnds,bool);
  vtkBooleanMacro(CapEnds,bool);

  vtkSetMacro(Arrow,bool);
  vtkGetMacro(Arrow,bool);
  vtkBooleanMacro(Arrow,bool);

  vtkSetMacro(LineStem,bool);
  vtkGetMacro(LineStem,bool);
  vtkBooleanMacro(LineStem,bool);

  vtkSetClampMacro(StemWidth,float,0.0,0.5);
  vtkGetMacro(StemWidth,float);

  vtkSetMacro(MakeHead,bool);
  vtkGetMacro(MakeHead,bool);
  vtkBooleanMacro(MakeHead,bool);

  vtkSetClampMacro(HeadSize,float,0.0,1.0);
  vtkGetMacro(HeadSize,float);

  vtkSetClampMacro(OriginOffset,float,-.5,+.5);
  vtkGetMacro(OriginOffset,float);

  vtkSetMacro(ConeHead,bool);
  vtkGetMacro(ConeHead,bool);
  vtkBooleanMacro(ConeHead,bool);

protected:
  vtkVectorGlyph();
  ~vtkVectorGlyph() {};

  virtual int RequestData(vtkInformation *,
                  vtkInformationVector **,
                  vtkInformationVector *) override;
  virtual int FillInputPortInformation(int port, vtkInformation *info) override;

  bool HighQuality;
  bool CapEnds;
  bool Arrow;
  bool LineStem;
  float StemWidth;
  bool MakeHead;
  float HeadSize;
  float OriginOffset;
  bool ConeHead;

private:
  vtkVectorGlyph(const vtkVectorGlyph&);
  void operator=(const vtkVectorGlyph&);
};

#endif


