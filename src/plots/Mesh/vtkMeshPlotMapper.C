#include "vtkMeshPlotMapper.h"

#include <vtkObjectFactory.h>


//---------------------------------------------------------------------
vtkAbstractObjectFactoryNewMacro( vtkMeshPlotMapper );

//---------------------------------------------------------------------
vtkMeshPlotMapper::vtkMeshPlotMapper()
{
    LinesColor[0] = LinesColor[1] = LinesColor[2] = 0.;
    PolysColor[0] = PolysColor[1] = PolysColor[2] = 0.;
    UsePolys = true;
    Opacity = 1.;
}

//---------------------------------------------------------------------
vtkMeshPlotMapper::~vtkMeshPlotMapper()
{
}

//---------------------------------------------------------------------
void
vtkMeshPlotMapper::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Lines Color: " << this->LinesColor[0] << ", "
     << this->LinesColor[1] << ", " << this->LinesColor[2] << endl; 
  os << indent << "Polys Color: " << this->PolysColor[0] << ", "
     << this->PolysColor[1] << ", " << this->PolysColor[2] << endl; 
}

//---------------------------------------------------------------------
bool
ColorsAreDifferent(double a[3], double b[3])
{
   return ((a[0] != b[0]) || 
           (a[1] != b[1]) || 
           (a[2] != b[2]));
}

//---------------------------------------------------------------------
void
vtkMeshPlotMapper::SetLinesColor(double rgb[3])
{
    if (ColorsAreDifferent(this->LinesColor, rgb))
    {
        this->LinesColor[0] = rgb[0];
        this->LinesColor[1] = rgb[1];
        this->LinesColor[2] = rgb[2];
        this->Modified();
    }
}

//---------------------------------------------------------------------
double *
vtkMeshPlotMapper::GetLinesColor()
{
  return this->LinesColor;
}

//---------------------------------------------------------------------
void
vtkMeshPlotMapper::SetPolysColor(double rgb[3])
{
    if (ColorsAreDifferent(this->PolysColor, rgb))
    {
        this->PolysColor[0] = rgb[0];
        this->PolysColor[1] = rgb[1];
        this->PolysColor[2] = rgb[2];
        this->Modified();
    }
}
//---------------------------------------------------------------------
double *
vtkMeshPlotMapper::GetPolysColor()
{
  return this->PolysColor;
}

