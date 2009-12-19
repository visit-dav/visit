/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkMesaStructuredGridMapper.cxx,v $
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
// Make sure this is first, so any includes of gl.h can be stoped if needed
#define VTK_IMPLEMENT_MESA_CXX

#include "MangleMesaInclude/gl_mangle.h"
#include "MangleMesaInclude/gl.h"

#include <math.h>
#include "vtkMesaStructuredGridMapper.h"
#include "vtkRenderWindow.h"
#include "vtkMesaProperty.h"
#include "vtkMesaCamera.h"
#include "vtkMesaLight.h"
#include "vtkCuller.h"
#include "vtkToolkits.h"


// make sure this file is included before the #define takes place
// so we don't get two vtkMesaStructuredGridMapper classes defined.
#include "vtkOpenGLStructuredGridMapper.h"
#include "vtkMesaStructuredGridMapper.h"

// Make sure vtkMesaStructuredGridMapper is a copy of vtkOpenGLStructuredGridMapper
// with vtkOpenGLStructuredGridMapper replaced with vtkMesaStructuredGridMapper
#define vtkOpenGLStructuredGridMapper vtkMesaStructuredGridMapper
#include "vtkOpenGLStructuredGridMapper.C"
#undef vtkOpenGLStructuredGridMapper
#undef VTK_IMPLEMENT_MESA_CXX
vtkCxxRevisionMacro(vtkMesaStructuredGridMapper, "$Revision: 1.12 $");
vtkStandardNewMacro(vtkMesaStructuredGridMapper);
