
#ifndef __vtkVisItQuad_h
#define __vtkVisItQuad_h

#include <vtkQuad.h>
#include <visit_vtk_exports.h>


class vtkTriangle;

class VISIT_VTK_API vtkVisItQuad : public vtkQuad
{
public:
  static vtkVisItQuad *New();
  vtkTypeRevisionMacro(vtkVisItQuad,vtkQuad);

  int IntersectWithLine(float p1[3], float p2[3], float tol, float& t,
                        float x[3], float pcoords[3], int& subId);


protected:
  vtkVisItQuad();
  ~vtkVisItQuad();

  vtkTriangle *Triangle;
private:
  vtkVisItQuad(const vtkVisItQuad&);  // Not implemented.
  void operator=(const vtkVisItQuad&);  // Not implemented.
};

#endif


