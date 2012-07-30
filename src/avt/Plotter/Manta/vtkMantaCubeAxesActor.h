//
//  The Cube Axes Actor used in VisIt is very slow in Manta, so overwrite it to not do anything:
//
#ifndef VTKMANTACUBEAXESACTOR_H
#define VTKMANTACUBEAXESACTOR_H


#include <vtkVisItCubeAxesActor.h>

class vtkMantaCubeAxesActor : public vtkVisItCubeAxesActor
{
public:
       vtkMantaCubeAxesActor() {}
       virtual ~vtkMantaCubeAxesActor() {}
       static vtkMantaCubeAxesActor *New();
       virtual int RenderOpaqueGeometry(vtkViewport*) {return 0;}
};


#endif
