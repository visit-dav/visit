/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkPolyDataSilhouette.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkDataSetGradient - computes scalar field gradient
// .SECTION Description
// vtkDataSetGradient Computes per cell gradient of point scalar field
// or per point gradient of cell scalar field.

// .SECTION Credits
// Contributed by Thierry Carrard from
// Commissariat a l'Energie Atomique (CEA),
// BP12, 91297 Arpajon, France

#ifndef VTK_DATA_SET_GRADIENT_H
#define VTK_DATA_SET_GRADIENT_H
/*-----------------------------------*
 *                LOVE               *
 *-----------------------------------*/
/*!
\brief Contains definition of class vtkDataSetGradient (component .VtkExt)
\author Thierry Carrard
\date 29 Novembre 2005

Modifications history :
*/

#ifndef SWIG
const static char * VTK_DATA_SET_GRADIENT_H_SCCS_ID = "%Z% DSSI/SNEC/LDDC %M%   %I%     %G%";
#endif /*SWIG*/

#include <vtkObjectFactory.h>
#include <vtkSetGet.h>
#include <vtkDataSetAlgorithm.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>

class vtkDataSetGradient : public vtkDataSetAlgorithm
{
  public:

    static vtkDataSetGradient* New();
    vtkTypeRevisionMacro(vtkDataSetGradient,vtkDataSetAlgorithm);
    ~vtkDataSetGradient();

    // Description:
    // Set/Get the name of computed vector array.
    vtkSetStringMacro(GradientArrayName);
    vtkGetStringMacro(GradientArrayName);

  protected:
    vtkDataSetGradient ();

    virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

    char* GradientArrayName;

  private:

    //! Unimplemented copy constructor
    vtkDataSetGradient (const vtkDataSetGradient &);

    //! Unimplemented operator
    vtkDataSetGradient & operator= (const vtkDataSetGradient &);
} ;

#endif /* VTK_DATA_SET_GRADIENT_H */
