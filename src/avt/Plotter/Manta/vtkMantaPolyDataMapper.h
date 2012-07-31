/*=========================================================================

Program:   Visualization Toolkit
Module:    $RCSfile: vtkMantaPolyDataMapper.h,v $

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*=========================================================================

Program:   VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
Module:    $RCSfile: vtkMantaPolyDataMapper.h,v $

Copyright (c) 2007, Los Alamos National Security, LLC

All rights reserved.

Copyright 2007. Los Alamos National Security, LLC.
This software was produced under U.S. Government contract DE-AC52-06NA25396
for Los Alamos National Laboratory (LANL), which is operated by
Los Alamos National Security, LLC for the U.S. Department of Energy.
The U.S. Government has rights to use, reproduce, and distribute this software.
NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY, LLC MAKES ANY WARRANTY,
EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY FOR THE USE OF THIS SOFTWARE.
If software is modified to produce derivative works, such modified software
should be clearly marked, so as not to confuse it with the version available
from LANL.

Additionally, redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following conditions
are met:
-   Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
-   Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
-   Neither the name of Los Alamos National Security, LLC, Los Alamos National
Laboratory, LANL, the U.S. Government, nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL LOS ALAMOS NATIONAL SECURITY, LLC OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkMantaPolyDataMapper -
// .SECTION Description
//
// .NAME vtkMantaPolyDataMapper - a PolyDataMapper for the Manta library
// .SECTION Description
// vtkMantaPolyDataMapper is a subclass of vtkPolyDataMapper.
// vtkMantaPolyDataMapper is a geometric PolyDataMapper for the Manta
// Raytracer library.

#ifndef __vtkMantaPolyDataMapper_h
#define __vtkMantaPolyDataMapper_h

//#include "vtkMantaConfigure.h"
#include "vtkPolyDataMapper.h"
#include "vtkMantaTexture.h"
#include "plotter_exports.h"
//#include <avtMapper.h>
//#include <AttributeSubject.h>
//#include <ColorAttribute.h>
//#include "MantaMoleculeAttributes.h"

class vtkSphereSource;
class vtkGlyph3D;
class vtkTubeFilter;
class vtkAppendPolyData;

//BTX
namespace Manta {
  class Mesh;
  class Group;
}
//ETX
class vtkCellArray;
class vtkPoints;
class vtkProperty;
class vtkRenderWindow;
class vtkMantaRenderer;
class vtkMantaManager;

class PLOTTER_API vtkMantaPolyDataMapper : public vtkPolyDataMapper
{
  public:
    static vtkMantaPolyDataMapper *New(){ return new vtkMantaPolyDataMapper; }
    //vtkTypeRevisionMacro(vtkMantaPolyDataMapper,vtkPolyDataMapper);
    //virtual void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Implement superclass render method.
    virtual void RenderPiece(vtkRenderer *ren, vtkActor *a) ;

    // Description:
    // Release any graphics resources that are being consumed by this mapper.
    // The parameter window could be used to determine which graphic
    // resources to release.
    void ReleaseGraphicsResources(vtkWindow *);

    // Description:
    // Draw method for Manta.
    virtual void Draw(vtkRenderer *ren, vtkActor *a);


    void AddSphere(double x, double y, double z, double radius, unsigned char r, unsigned char g, unsigned char b);
    void AddCylinder(double x, double y, double z, double x1, double y1, double z1, double radius, unsigned char r, unsigned char g, unsigned char b);


   // void SetMoleculeAtts(const MantaMoleculeAttributes& atts) { molecule_atts = atts; }

  protected:
    vtkMantaPolyDataMapper();
    ~vtkMantaPolyDataMapper();

   // void DrawMolecules(vtkPolyData *data, Manta::Group* sphereGroup,
     //   const MantaMoleculeAttributes &atts);

    //BTX
    void DrawPolygons(vtkPolyData *, vtkPoints *,
        Manta::Mesh *, Manta::Group *, Manta::Group *);
    void DrawTStrips(vtkPolyData *, vtkPoints *,
        Manta::Mesh *, Manta::Group *, Manta::Group *);
    //ETX

  private:
    //vtkMantaPolyDataMapper(const vtkMantaPolyDataMapper&); // Not implemented.
    //void operator=(const vtkMantaPolyDataMapper&); // Not implemented.

    vtkMantaManager *MantaManager;

    vtkMantaTexture* InternalColorTexture;
    int Representation;
    double PointSize;
    double LineWidth;
    bool CellScalarColor;


  //  MantaMoleculeAttributes molecule_atts;
    int            numcolors;
    unsigned char *colors;
    std::string    colortablename;
    bool           ct_is_discrete;

    //BTX
    class Helper;
    Helper *MyHelper;
    //ETX
    //
    struct SphereData
    {
      public:
        SphereData() {}
        SphereData(double x_, double y_, double z_, double radius_, unsigned char r_, unsigned char g_, unsigned char b_) {x=x_;y=y_;z=z_;radius=radius_;r=r_;g=g_;b=b_;}
        double x,y,z,radius;
        unsigned char r,g,b;
    };
    struct CylinderData
    {
      public:
        CylinderData() {}
        CylinderData(double x1_, double y1_, double z1_, double x2_, double y2_, double z2_, double radius_, unsigned char r_, unsigned char g_, unsigned char b_) {p1[0]=x1_;p1[1]=y1_;p1[2]=z1_;p2[0]=x2_;p2[1]=y2_;p2[2]=z2_;radius=radius_;r=r_;g=g_;b=b_;}
        double p1[3],p2[3],radius;
        unsigned char r,g,b;
    };

    std::vector<SphereData> new_spheres;
    std::vector<CylinderData> new_cylinders;
};

#endif
