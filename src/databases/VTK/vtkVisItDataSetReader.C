/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItDataSetReader.cxx,v $
  Language:  C++
  Date:      $Date: 2003/07/22 19:27:45 $
  Version:   $Revision: 1.62 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItDataSetReader.h"

#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkVisItPolyDataReader.h"
#include "vtkRectilinearGrid.h"
#include "vtkVisItRectilinearGridReader.h"
#include "vtkStructuredGrid.h"
#include "vtkVisItStructuredGridReader.h"
#include "vtkStructuredPoints.h"
#include "vtkVisItStructuredPointsReader.h"
#include "vtkUnstructuredGrid.h"
#include "vtkVisItUnstructuredGridReader.h"

vtkCxxRevisionMacro(vtkVisItDataSetReader, "$Revision: 1.62 $");
vtkStandardNewMacro(vtkVisItDataSetReader);

vtkVisItDataSetReader::vtkVisItDataSetReader()
{
}

vtkVisItDataSetReader::~vtkVisItDataSetReader()
{
}

vtkDataSet * vtkVisItDataSetReader::GetOutput() 
{
  // check to see if an execute is necessary.
  if (this->Outputs && this->Outputs[0] && 
      this->Outputs[0]->GetUpdateTime() > this->GetMTime())
    {
    return (vtkDataSet *)(this->Outputs[0]);
    }
  
  // The filename might have changed (changing the output).
  // We need to re execute.
  if (this->GetFileName() == NULL && 
      (this->GetReadFromInputString() == 0 || 
       (this->GetInputArray() == NULL && this->GetInputString() == NULL)))
    {
    vtkWarningMacro(<< "FileName must be set");
    return (vtkDataSet *) NULL;
    }

  this->Execute();
  if (this->Outputs == NULL)
    {
    return NULL;
    }
  else
    {
    return (vtkDataSet *)this->Outputs[0];
    }
}

void vtkVisItDataSetReader::Execute()
{
  vtkDataObject *output;
  
  vtkDebugMacro(<<"Reading vtk dataset...");

  switch (this->ReadOutputType())
    {
    case VTK_POLY_DATA:
      {
      vtkVisItPolyDataReader *preader = vtkVisItPolyDataReader::New();
      preader->SetFileName(this->GetFileName());
      preader->SetInputArray(this->GetInputArray());
      preader->SetInputString(this->GetInputString(),
                              this->GetInputStringLength());
      preader->SetReadFromInputString(this->GetReadFromInputString());
      preader->SetScalarsName(this->GetScalarsName());
      preader->SetVectorsName(this->GetVectorsName());
      preader->SetNormalsName(this->GetNormalsName());
      preader->SetTensorsName(this->GetTensorsName());
      preader->SetTCoordsName(this->GetTCoordsName());
      preader->SetLookupTableName(this->GetLookupTableName());
      preader->SetFieldDataName(this->GetFieldDataName());
      preader->SetReadAllScalars(this->GetReadAllScalars());
      preader->SetReadAllVectors(this->GetReadAllVectors());
      preader->SetReadAllTensors(this->GetReadAllTensors());
      preader->Update();
      // Can we use the old output?
      output = this->Outputs ? this->Outputs[0] : NULL;
      if (output && strcmp(output->GetClassName(), "vtkPolyData") == 0)
        {
        output->ShallowCopy(preader->GetOutput());
        }
      else
        {
        // Hack to make sure that the object is not modified
        // with SetNthOutput. Otherwise, extra executions occur.
        vtkTimeStamp ts = this->MTime;
        this->SetNthOutput(0, preader->GetOutput());
        this->MTime = ts;
        }
      preader->Delete();
      return;
      }
    case VTK_STRUCTURED_POINTS:
      {
      vtkVisItStructuredPointsReader *preader = vtkVisItStructuredPointsReader::New();
      preader->SetFileName(this->GetFileName());
      preader->SetInputArray(this->GetInputArray());
      preader->SetInputString(this->GetInputString(),
                              this->GetInputStringLength());
      preader->SetReadFromInputString(this->GetReadFromInputString());
      preader->SetScalarsName(this->GetScalarsName());
      preader->SetVectorsName(this->GetVectorsName());
      preader->SetNormalsName(this->GetNormalsName());
      preader->SetTensorsName(this->GetTensorsName());
      preader->SetTCoordsName(this->GetTCoordsName());
      preader->SetLookupTableName(this->GetLookupTableName());
      preader->SetFieldDataName(this->GetFieldDataName());
      preader->SetReadAllScalars(this->GetReadAllScalars());
      preader->SetReadAllVectors(this->GetReadAllVectors());
      preader->SetReadAllTensors(this->GetReadAllTensors());
      preader->Update();
      // Can we use the old output?
      output = this->Outputs ? this->Outputs[0] : NULL;
      if (output && strcmp(output->GetClassName(), "vtkStructuredPoints") == 0)
        {
        output->ShallowCopy(preader->GetOutput());
        }
      else
        {
        // Hack to make sure that the object is not modified
        // with SetNthOutput. Otherwise, extra executions occur.
        vtkTimeStamp ts = this->MTime;
        this->SetNthOutput(0, preader->GetOutput());
        this->MTime = ts;
        }
      preader->Delete();
      return;
      }
    case VTK_STRUCTURED_GRID:
      {
      vtkVisItStructuredGridReader *preader = vtkVisItStructuredGridReader::New();
      preader->SetFileName(this->GetFileName());
      preader->SetInputArray(this->GetInputArray());
      preader->SetInputString(this->GetInputString(),
                              this->GetInputStringLength());
      preader->SetReadFromInputString(this->GetReadFromInputString());
      preader->SetScalarsName(this->GetScalarsName());
      preader->SetVectorsName(this->GetVectorsName());
      preader->SetNormalsName(this->GetNormalsName());
      preader->SetTensorsName(this->GetTensorsName());
      preader->SetTCoordsName(this->GetTCoordsName());
      preader->SetLookupTableName(this->GetLookupTableName());
      preader->SetFieldDataName(this->GetFieldDataName());
      preader->SetReadAllScalars(this->GetReadAllScalars());
      preader->SetReadAllVectors(this->GetReadAllVectors());
      preader->SetReadAllTensors(this->GetReadAllTensors());
      preader->Update();
      // Can we use the old output?
      output = this->Outputs ? this->Outputs[0] : NULL;
      if (output && strcmp(output->GetClassName(), "vtkStructuredGrid") == 0)
        {
        output->ShallowCopy(preader->GetOutput());
        }
      else
        {
        // Hack to make sure that the object is not modified
        // with SetNthOutput. Otherwise, extra executions occur.
        vtkTimeStamp ts = this->MTime;
        this->SetNthOutput(0, preader->GetOutput());
        this->MTime = ts;
        }
      preader->Delete();
      return;
      }
    case VTK_RECTILINEAR_GRID:
      {
      vtkVisItRectilinearGridReader *preader = vtkVisItRectilinearGridReader::New();
      preader->SetFileName(this->GetFileName());
      preader->SetInputArray(this->GetInputArray());
      preader->SetInputString(this->GetInputString(),
                              this->GetInputStringLength());
      preader->SetReadFromInputString(this->GetReadFromInputString());
      preader->SetScalarsName(this->GetScalarsName());
      preader->SetVectorsName(this->GetVectorsName());
      preader->SetNormalsName(this->GetNormalsName());
      preader->SetTensorsName(this->GetTensorsName());
      preader->SetTCoordsName(this->GetTCoordsName());
      preader->SetLookupTableName(this->GetLookupTableName());
      preader->SetFieldDataName(this->GetFieldDataName());
      preader->SetReadAllScalars(this->GetReadAllScalars());
      preader->SetReadAllVectors(this->GetReadAllVectors());
      preader->SetReadAllTensors(this->GetReadAllTensors());
      preader->Update();
      // Can we use the old output?
      output = this->Outputs ? this->Outputs[0] : NULL;
      if (output && strcmp(output->GetClassName(), "vtkRectilinearGrid") == 0)
        {
        output->ShallowCopy(preader->GetOutput());
        }
      else
        {
        // Hack to make sure that the object is not modified
        // with SetNthOutput. Otherwise, extra executions occur.
        vtkTimeStamp ts = this->MTime;
        this->SetNthOutput(0, preader->GetOutput());
        this->MTime = ts;
        }
      preader->Delete();
      return;
      }
    case VTK_UNSTRUCTURED_GRID:
      {
      vtkVisItUnstructuredGridReader *preader = vtkVisItUnstructuredGridReader::New();
      preader->SetFileName(this->GetFileName());
      preader->SetInputArray(this->GetInputArray());
      preader->SetInputString(this->GetInputString(),
                              this->GetInputStringLength());
      preader->SetReadFromInputString(this->GetReadFromInputString());
      preader->SetScalarsName(this->GetScalarsName());
      preader->SetVectorsName(this->GetVectorsName());
      preader->SetNormalsName(this->GetNormalsName());
      preader->SetTensorsName(this->GetTensorsName());
      preader->SetTCoordsName(this->GetTCoordsName());
      preader->SetLookupTableName(this->GetLookupTableName());
      preader->SetFieldDataName(this->GetFieldDataName());
      preader->SetReadAllScalars(this->GetReadAllScalars());
      preader->SetReadAllVectors(this->GetReadAllVectors());
      preader->SetReadAllTensors(this->GetReadAllTensors());
      preader->Update();
      // Can we use the old output?
      output = this->Outputs ? this->Outputs[0] : NULL;
      if (output && strcmp(output->GetClassName(), "vtkUnstructuredGrid") == 0)
        {
        output->ShallowCopy(preader->GetOutput());
        }
      else
        {
        // Hack to make sure that the object is not modified
        // with SetNthOutput. Otherwise, extra executions occur.
        vtkTimeStamp ts = this->MTime;
        this->SetNthOutput(0, preader->GetOutput());
        this->MTime = ts;
        }
      preader->Delete();
      return;
      }
    default:
        vtkErrorMacro("Could not read file " << this->FileName);
    }
}


int vtkVisItDataSetReader::ReadOutputType()
{
  char line[256];
  
  vtkDebugMacro(<<"Reading vtk dataset...");

  if (!this->OpenVTKFile() || !this->ReadHeader())
    {
    return -1;
    }

  // Determine dataset type
  //
  if (!this->ReadString(line))
    {
    //vtkErrorMacro(<< "Premature EOF reading dataset keyword");
    return -1;
    }

  if ( !strncmp(this->LowerCase(line),"dataset",(unsigned long)7) )
    {
    // See if type is recognized.
    //
    if (!this->ReadString(line))
      {
      //vtkErrorMacro(<< "Premature EOF reading type");
      this->CloseVTKFile ();
      return -1;
      }

    this->CloseVTKFile();
    if ( ! strncmp(this->LowerCase(line),"polydata",8) )
      {
      return VTK_POLY_DATA;
      }
    else if ( ! strncmp(line,"structured_points",17) )
      {
      return VTK_STRUCTURED_POINTS;
      }
    else if ( ! strncmp(line,"structured_grid",15) )
      {
      return VTK_STRUCTURED_GRID;
      }
    else if ( ! strncmp(line,"rectilinear_grid",16) )
      {
      return VTK_RECTILINEAR_GRID;
      }
    else if ( ! strncmp(line,"unstructured_grid",17) )
      {
      return VTK_UNSTRUCTURED_GRID;
      }
    else
      {
      //vtkErrorMacro(<< "Cannot read dataset type: " << line);
      return -1;
      }
    }

  else if ( !strncmp(this->LowerCase(line),"field",(unsigned long)5) )
    {
    //vtkErrorMacro(<<"This object can only read datasets, not fields");
    }
  
  else
    {
    //vtkErrorMacro(<<"Expecting DATASET keyword, got " << line << " instead");
    }

  return -1;
}





vtkPolyData *vtkVisItDataSetReader::GetPolyDataOutput()
{
  return vtkPolyData::SafeDownCast(this->GetOutput());
}

vtkStructuredPoints *vtkVisItDataSetReader::GetStructuredPointsOutput() 
{
  return vtkStructuredPoints::SafeDownCast(this->GetOutput());
}

vtkStructuredGrid *vtkVisItDataSetReader::GetStructuredGridOutput() 
{
  return vtkStructuredGrid::SafeDownCast(this->GetOutput());
}

vtkUnstructuredGrid *vtkVisItDataSetReader::GetUnstructuredGridOutput() 
{
  return vtkUnstructuredGrid::SafeDownCast(this->GetOutput());
}

vtkRectilinearGrid *vtkVisItDataSetReader::GetRectilinearGridOutput() 
{
  return vtkRectilinearGrid::SafeDownCast(this->GetOutput());
}


//----------------------------------------------------------------------------
void vtkVisItDataSetReader::Update()
{
  if (this->GetOutput())
    {
    this->GetOutput()->Update();
    }
}

void vtkVisItDataSetReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

vtkDataSet *vtkVisItDataSetReader::GetOutput(int idx)
{
  return static_cast<vtkDataSet *>(this->vtkSource::GetOutput(idx)); 
}
