// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkVisItFullFrameMapper2D.h"
#include "vtkObjectFactory.h"

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkVisItFullFrameMapper2D);

//-----------------------------------------------------------------------------
vtkVisItFullFrameMapper2D::vtkVisItFullFrameMapper2D()
{
    this->FullFrameScaling[0] = 1.;
    this->FullFrameScaling[1] = 1.;
    this->FullFrameScaling[2] = 1.;
    this->UseFullFrameScaling = false;
}

//-----------------------------------------------------------------------------
vtkVisItFullFrameMapper2D::~vtkVisItFullFrameMapper2D()
{
}

//-----------------------------------------------------------------------------
bool
vtkVisItFullFrameMapper2D::SetFullFrameScaling(bool useScaling,
    const double *scale)
{
    bool modified = ((useScaling != this->UseFullFrameScaling) ||
                     (scale[0] != this->FullFrameScaling[0]) ||
                     (scale[1] != this->FullFrameScaling[1]) ||
                     (scale[2] != this->FullFrameScaling[2]));
    
    this->UseFullFrameScaling = useScaling;
    
    if (useScaling)
    {   
        this->FullFrameScaling[0] = scale[0];
        this->FullFrameScaling[1] = scale[1];
        this->FullFrameScaling[2] = scale[2];
    }
    else
    {   
        this->FullFrameScaling[0] = 1.;
        this->FullFrameScaling[1] = 1.;
        this->FullFrameScaling[2] = 1.;
    }
    
    if (modified)
    {   
        this->Modified();
    }
    
    return modified;
}

//-----------------------------------------------------------------------------
bool
vtkVisItFullFrameMapper2D::SetFullFrameScaling(const double *scale)
{   
    this->SetFullFrameScaling(this->UseFullFrameScaling, scale);
    return this->UseFullFrameScaling;
}

//-----------------------------------------------------------------------------
void vtkVisItFullFrameMapper2D::PrintSelf(ostream& os, vtkIndent indent) 
{
    this->Superclass::PrintSelf(os,indent);

    os << indent << "FullFrameScaling: \n";
    os << indent << "  X: " << this->FullFrameScaling[0] << "\n"; 
    os << indent << "  Y: " << this->FullFrameScaling[1] << "\n"; 
    os << indent << "  Z: " << this->FullFrameScaling[2] << "\n"; 

    os << indent << "UseFullFrameScaling: " << this->UseFullFrameScaling << "\n";
}
