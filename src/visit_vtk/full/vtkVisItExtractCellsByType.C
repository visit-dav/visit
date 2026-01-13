// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.


#include "vtkVisItExtractCellsByType.h"

#include <visit-config.h>

#include <vtkCellType.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkVisItExtractCellsByType);


//----------------------------------------------------------------------------
vtkVisItExtractCellsByType::vtkVisItExtractCellsByType()
{
}

//----------------------------------------------------------------------------
vtkVisItExtractCellsByType::~vtkVisItExtractCellsByType()
{
}

// vtkExtractCellsByType::AddAllCellTypes when used in conjunction with
// vtkExtractCellsByType::RemoveCellType (for instance, to extract all
// cell types except lines), actually prevents any cells from being extracted.
// This is due to use of VTK_ANY_CELL_TYPE in the AddAllCellTypes method.
//
// Not all 'i' in the for loop below will be valid cell types, but that
// should not interfere with the the execution of the parent class, except
// perhaps slow down the check for wether or not a given cell should be extracted.

void vtkVisItExtractCellsByType::AddAllCellTypes2()
{
  for (unsigned int i = 0; i < VTK_NUMBER_OF_CELL_TYPES; ++i)
      this->AddCellType(i);
}

