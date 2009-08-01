/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItMesaPolyDataMapper.cxx,v $
  Language:  C++
  Date:      $Date: 2002/06/03 15:59:53 $
  Version:   $Revision: 1.12 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// Make sure this is first, so any includes of gl.h can be stopped if needed
#define VTK_IMPLEMENT_MESA_CXX

#include <math.h>
#include <avtGLEWInitializer.h>

#include "vtkVisItMesaPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkMesaProperty.h"
#include "vtkMesaCamera.h"
#include "vtkMesaLight.h"
#include "vtkCuller.h"
#include "vtkToolkits.h"


// make sure this file is included before the #define takes place
// so we don't get two vtkVisItMesaPolyDataMapper classes defined.
#include "vtkVisItOpenGLPolyDataMapper.h"
#include "vtkVisItMesaPolyDataMapper.h"

// Make sure vtkVisItMesaPolyDataMapper is a copy of vtkVisItOpenGLPolyDataMapper
// with vtkVisItOpenGLPolyDataMapper replaced with vtkVisItMesaPolyDataMapper
#define vtkVisItOpenGLPolyDataMapper vtkVisItMesaPolyDataMapper
#include "vtkVisItOpenGLPolyDataMapper.C"
#undef vtkVisItOpenGLPolyDataMapper

vtkCxxRevisionMacro(vtkVisItMesaPolyDataMapper, "$Revision: 1.12 $");
vtkStandardNewMacro(vtkVisItMesaPolyDataMapper);
