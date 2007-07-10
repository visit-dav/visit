/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkThreshold.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkThreshold - extracts cells where scalar value in cell satisfies threshold criterion
// .SECTION Description
// vtkThreshold is a filter that extracts cells from any dataset type that
// satisfy a threshold criterion. A cell satisfies the criterion if the
// scalar value of (every or any) point satisfies the criterion. The
// criterion can take three forms: 1) greater than a particular value; 2)
// less than a particular value; or 3) between two values. The output of this
// filter is an unstructured grid.
//
// Note that scalar values are available from the point and cell attribute
// data.  By default, point data is used to obtain scalars, but you can
// control this behavior. See the AttributeMode ivar below.
//
// By default only the first scalar value is used in the decision. Use the ComponentMode
// and SelectedComponent ivars to control this behavior.

// .SECTION See Also
// vtkThresholdPoints vtkThresholdTextureCoords

// ----------------------------------------------------------------------------
//  Modifications:
//    Jeremy Meredith, Tue Aug 22 16:20:41 EDT 2006
//    Taken from 5.0.0.a vtkThreshold source, renamed to vtkEnumThreshold, and
//    made it work on an enumerated scalar selection.
//
//    Mark C. Miller and Jeremy Meredith, Tue Jul 10 08:45:44 PDT 2007
//    Added minEnumerationValue data member so this class can handle negative
//    enumeration values.


#ifndef __vtkEnumThreshold_h
#define __vtkEnumThreshold_h
#include <visit_vtk_light_exports.h>

#include "vtkUnstructuredGridAlgorithm.h"

#include <vector>
#include <string>

class vtkDataArray;

class VISIT_VTK_LIGHT_API vtkEnumThreshold : public vtkUnstructuredGridAlgorithm
{
  public:
    static vtkEnumThreshold *New();
    vtkTypeRevisionMacro(vtkEnumThreshold,vtkUnstructuredGridAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent);

    void SetEnumerationValues(const std::vector<int>&);
    void SetEnumerationSelection(const std::vector<bool>&);

  protected:
    vtkEnumThreshold();
    ~vtkEnumThreshold();

    // Usual data generation method
    virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

    virtual int FillInputPortInformation(int port, vtkInformation *info);
  
    int EvaluateComponents( vtkDataArray *scalars, vtkIdType id );
  
    std::vector<int> enumerationValues;
    std::vector<bool> enumerationSelection;
    int            maxEnumerationValue;
    int            minEnumerationValue;
    unsigned char *enumerationMap;

  private:
    vtkEnumThreshold(const vtkEnumThreshold&);  // Not implemented.
    void operator=(const vtkEnumThreshold&);  // Not implemented.
};

#endif
