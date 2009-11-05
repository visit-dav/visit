/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMesaRectilinearGridMapper.cxx,v $
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

#include "vtkMesaRectilinearGridMapper.h"
#include "vtkRenderWindow.h"
#include "vtkMesaProperty.h"
#include "vtkMesaCamera.h"
#include "vtkMesaLight.h"
#include "vtkCuller.h"
#include "vtkToolkits.h"


// make sure this file is included before the #define takes place
// so we don't get two vtkMesaRectilinearGridMapper classes defined.
#include "vtkOpenGLRectilinearGridMapper.h"
#include "vtkMesaRectilinearGridMapper.h"

// Make sure vtkMesaRectilinearGridMapper is a copy of vtkOpenGLRectilinearGridMapper
// with vtkOpenGLRectilinearGridMapper replaced with vtkMesaRectilinearGridMapper
#define vtkOpenGLRectilinearGridMapper vtkMesaRectilinearGridMapper
#include "vtkOpenGLRectilinearGridMapper.C"
#undef vtkOpenGLRectilinearGridMapper

vtkCxxRevisionMacro(vtkMesaRectilinearGridMapper, "$Revision: 1.12 $");
vtkStandardNewMacro(vtkMesaRectilinearGridMapper);
