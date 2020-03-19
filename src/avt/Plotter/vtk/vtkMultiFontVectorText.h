// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTK_MULTI_FONT_VECTOR_TEXT_H
#define VTK_MULTI_FONT_VECTOR_TEXT_H
#include <vtkPolyDataAlgorithm.h>

class vtkMultiFontVectorText : public vtkPolyDataAlgorithm
{
public:
  static vtkMultiFontVectorText *New();

#if 0
  // Leave out for now. They cause unresolved __ZTV22vtkMultiFontVectorText
  // with g++ for reasons I don't understand. That's the vtable for the object.
  vtkTypeMacro(vtkMultiFontVectorText,vtkPolyDataAlgorithm);
#endif
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set/Get the text to be drawn.
  vtkSetStringMacro(Text);
  vtkGetStringMacro(Text);

  // Description:
  // Set/Get the font used to generate the polydata
  vtkSetMacro(FontFamily,int);
  vtkGetMacro(FontFamily,int);
  void SetFontFamilyToArial() {this->SetFontFamily(VTK_ARIAL);}
  void SetFontFamilyToCourier() {this->SetFontFamily(VTK_COURIER);}
  void SetFontFamilyToTimes() {this->SetFontFamily(VTK_TIMES);}

  // Description:
  // Set/Get the Bold value for the font.
  vtkSetMacro(Bold,bool);
  vtkGetMacro(Bold,bool);

  // Description:
  // Set/Get the Italic value for the font.
  vtkSetMacro(Italic,bool);
  vtkGetMacro(Italic,bool);

protected:
  vtkMultiFontVectorText();
  ~vtkMultiFontVectorText();

  int RequestData(vtkInformation *,
                  vtkInformationVector **,
                  vtkInformationVector *) override;

  char *Text;
  int   FontFamily;
  bool  Bold;
  bool  Italic;
};

#endif
