// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkVisItDataSetMapper.h"

#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>

using std::string;

vtkStandardNewMacro(vtkVisItDataSetMapper);


static vtkVisItDataSetMapper::RenderingMode rMode = vtkVisItDataSetMapper::RENDERING_MODE_NORMAL;

std::string
RenderModeAsString()
{
  string rModeString;
  switch(rMode)
  {
  case vtkVisItDataSetMapper::RENDERING_MODE_LUMINANCE:
        rModeString="Luminance"; 
        break;
  case vtkVisItDataSetMapper::RENDERING_MODE_VALUE:
        rModeString="Value"; 
        break;
  case vtkVisItDataSetMapper::RENDERING_MODE_NORMAL:
    default:
        rModeString="Normal"; 
  }
  return rModeString;
}


vtkVisItDataSetMapper::vtkVisItDataSetMapper()
{
    this->allwhite = nullptr;
    this->grayscale = nullptr;
}


vtkVisItDataSetMapper::~vtkVisItDataSetMapper()
{
    if(this->allwhite)
        this->allwhite->Delete();
    if(this->grayscale)
        this->grayscale->Delete();
}

void
vtkVisItDataSetMapper::SetRenderingMode(vtkVisItDataSetMapper::RenderingMode m)
{
    rMode = m;
}

vtkVisItDataSetMapper::RenderingMode
vtkVisItDataSetMapper::GetRenderingMode()
{
    return rMode;
}



//
//  Sets up vtkLookupTables for LUMINANCE and VALUE rendering modes
//

void 
vtkVisItDataSetMapper::Render(vtkRenderer *ren, vtkActor *act)
{
    // make sure that we've been properly initialized
    //
    if ( !this->GetInput() )
    {
        vtkErrorMacro(<< "No input!\n");
        return;
    }  

    // Set up for LUMINANCE or VALUE render modes 
    if(rMode == RENDERING_MODE_LUMINANCE)
    {
        if(this->allwhite == NULL)
        {
            this->allwhite = vtkLookupTable::New();
            this->allwhite->SetNumberOfColors(10);
            for(int i = 0; i < 10; ++i)
                this->allwhite->SetTableValue(i, 1.,1.,1.,1.);
            this->allwhite->SetRange(this->GetScalarRange());
        }
        this->SetLookupTable(this->allwhite);
    }
    else if(rMode == RENDERING_MODE_VALUE)
    {
        if(this->grayscale == NULL)
        {
            this->grayscale = vtkLookupTable::New();
            this->grayscale->SetNumberOfColors(1024);
            for(int i = 0; i < 1024; ++i)
            {
                double t = double(i)/double(1024-1);
                this->grayscale->SetTableValue(i, t,t,t,1.);
            }
            this->grayscale->SetRange(this->GetScalarRange());
        }
        this->SetLookupTable(this->grayscale);
    }

    // Let the superclass handle the actual rendering 
    this->Superclass::Render(ren,act);
}

void
vtkVisItDataSetMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "RenderMode: (" << RenderModeAsString() << ")\n" << endl;
}

