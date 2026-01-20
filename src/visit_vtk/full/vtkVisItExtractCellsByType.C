// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.


#include "vtkVisItExtractCellsByType.h"

#include <visit-config.h>

#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedCharArray.h>
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

//----------------------------------------------------------------------------
//  Calls AddCellType for all supported cell types of the input dataset
//  except for VTK_LINE and VTK_POLY_LINE.
//
void
vtkVisItExtractCellsByType::AddAllButLines(vtkDataSet *inDS)
{
    // want to extract everything except VTK_LINE, VTK_POLY_LINE
    if(inDS->GetDataObjectType() == VTK_POLY_DATA)
    {
        this->AddCellType(VTK_VERTEX);
        this->AddCellType(VTK_POLY_VERTEX);
        this->AddCellType(VTK_TRIANGLE);
        this->AddCellType(VTK_TRIANGLE_STRIP);
        this->AddCellType(VTK_QUAD);
        this->AddCellType(VTK_POLYGON);
    }
    else if(inDS->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnsignedCharArray *distinctCellTypes = static_cast<vtkUnstructuredGrid*>(inDS)->GetDistinctCellTypesArray();
        for(vtkIdType i = 0; i < distinctCellTypes->GetNumberOfTuples(); ++i)
        {
            unsigned char dct = distinctCellTypes->GetValue(i);
            if(dct != VTK_LINE && dct != VTK_POLY_LINE)
                this->AddCellType(dct);
        }
    }
}


//  Calls AddCellType for all supported cell types of the input dataset
//  except for VTK_VERTEX and VTK_POLY_VERTEX.
//
void
vtkVisItExtractCellsByType::AddAllButVerts(vtkDataSet *inDS)
{
    // want to extract everything except VTK_VERTEX, VTK_POLY_VERTEX
    if(inDS->GetDataObjectType() == VTK_POLY_DATA)
    {
        this->AddCellType(VTK_LINE);
        this->AddCellType(VTK_POLY_LINE);
        this->AddCellType(VTK_TRIANGLE);
        this->AddCellType(VTK_TRIANGLE_STRIP);
        this->AddCellType(VTK_QUAD);
        this->AddCellType(VTK_POLYGON);
    }
    else if(inDS->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnsignedCharArray *distinctCellTypes = static_cast<vtkUnstructuredGrid*>(inDS)->GetDistinctCellTypesArray();
        for(vtkIdType i = 0; i < distinctCellTypes->GetNumberOfTuples(); ++i)
        {
            unsigned char dct = distinctCellTypes->GetValue(i);
            if(dct != VTK_VERTEX && dct != VTK_POLY_VERTEX)
                this->AddCellType(dct);
        }
    }
}
