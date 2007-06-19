/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItDataSetReader.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkVisItDataSetReader.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkVisItPolyDataReader.h"
#include "vtkRectilinearGrid.h"
#include "vtkVisItRectilinearGridReader.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"
#include "vtkVisItStructuredGridReader.h"
#include "vtkStructuredPoints.h"
#include "vtkVisItStructuredPointsReader.h"
#include "vtkUnstructuredGrid.h"
#include "vtkVisItUnstructuredGridReader.h"

vtkCxxRevisionMacro(vtkVisItDataSetReader, "$Revision: 1.70 $");
vtkStandardNewMacro(vtkVisItDataSetReader);

vtkVisItDataSetReader::vtkVisItDataSetReader()
{
}

vtkVisItDataSetReader::~vtkVisItDataSetReader()
{
}

int vtkVisItDataSetReader::RequestDataObject(
  vtkInformation *,
  vtkInformationVector** vtkNotUsed(inputVector) , 
  vtkInformationVector* outputVector)
{
  if (this->GetFileName() == NULL &&
      (this->GetReadFromInputString() == 0 ||
       (this->GetInputArray() == NULL && this->GetInputString() == NULL)))
    {
    vtkWarningMacro(<< "FileName must be set");
    return 0;
    }

  int outputType = this->ReadOutputType();

  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkDataSet *output = vtkDataSet::SafeDownCast(
    info->Get(vtkDataObject::DATA_OBJECT()));

  if (output && (output->GetDataObjectType() == outputType))
    {
    return 1;
    }

  if (!output || output->GetDataObjectType() != outputType) 
    {
    switch (outputType)
      {
      case VTK_POLY_DATA:
        output = vtkPolyData::New();
        break;
      case VTK_STRUCTURED_POINTS:
        output = vtkStructuredPoints::New();
        break;
      case VTK_STRUCTURED_GRID:
        output = vtkStructuredGrid::New();
        break;
      case VTK_RECTILINEAR_GRID:
        output = vtkRectilinearGrid::New();
        break;
      case VTK_UNSTRUCTURED_GRID:
        output = vtkUnstructuredGrid::New();
        break;
      default:
        return 0;
      }
    
    this->GetExecutive()->SetOutputData(0, output);
    output->Delete();
    this->GetOutputPortInformation(0)->Set(
      vtkDataObject::DATA_EXTENT_TYPE(), output->GetExtentType());
    }

  return 1;
}

int vtkVisItDataSetReader::RequestInformation(
  vtkInformation *,
  vtkInformationVector **,
  vtkInformationVector *outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  if (this->GetFileName() == NULL &&
      (this->GetReadFromInputString() == 0 ||
       (this->GetInputArray() == NULL && this->GetInputString() == NULL)))
    {
    vtkWarningMacro(<< "FileName must be set");
    return 0;
    }

  vtkVisItDataReader *reader = 0;
  int retVal;
  switch (this->ReadOutputType())
    {
    case VTK_POLY_DATA:
      reader = vtkVisItPolyDataReader::New();
      break;
    case VTK_STRUCTURED_POINTS:
      reader = vtkVisItStructuredPointsReader::New();
      break;
    case VTK_STRUCTURED_GRID:
      reader = vtkVisItStructuredGridReader::New();
      break;
    case VTK_RECTILINEAR_GRID:
      reader = vtkVisItRectilinearGridReader::New();
      break;
    case VTK_UNSTRUCTURED_GRID:
      reader = vtkVisItUnstructuredGridReader::New();
      break;
    default:
      reader = NULL;
    }

  if (reader)
    {
    reader->SetFileName(this->GetFileName());
    reader->SetReadFromInputString(this->GetReadFromInputString());
    reader->SetInputArray(this->GetInputArray());
    reader->SetInputString(this->GetInputString());
    retVal = reader->ReadMetaData(outInfo);
    reader->Delete();
    return retVal;
    }
  return 1;
}


// ****************************************************************************
//  Method: vtkVisItDataSetReader::RequestData
//
//  Purpose:
//    Copied from VTK source.
//
//  Modified:
//    Dave Bremer.  Mon Jun 18 17:51:16 PDT 2007
//    Changed the way type checking happens for vtkStructuredGrid and 
//    vtkRectilinearGrid, for which I created relpacement subclasses
//    called vtkVisItStructuredGrid and vtkVisItRectilinearGrid.
// ****************************************************************************
int vtkVisItDataSetReader::RequestData(
  vtkInformation *,
  vtkInformationVector **,
  vtkInformationVector *outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkDataObject *output = outInfo->Get(vtkDataObject::DATA_OBJECT());
  
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
      preader->SetReadAllNormals(this->GetReadAllNormals());
      preader->SetReadAllTensors(this->GetReadAllTensors());
      preader->SetReadAllColorScalars(this->GetReadAllColorScalars());
      preader->SetReadAllTCoords(this->GetReadAllTCoords());
      preader->SetReadAllFields(this->GetReadAllFields());
      preader->Update();
      // Can we use the old output?
      if(!(output && strcmp(output->GetClassName(), "vtkPolyData") == 0))
        {
        // Hack to make sure that the object is not modified
        // with SetNthOutput. Otherwise, extra executions occur.
        vtkTimeStamp ts = this->MTime;
        output = vtkPolyData::New();
        this->GetExecutive()->SetOutputData(0, output);
        output->Delete();
        this->MTime = ts;
        }
      output->ShallowCopy(preader->GetOutput());
      preader->Delete();
      return 1;
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
      preader->SetReadAllNormals(this->GetReadAllNormals());
      preader->SetReadAllTensors(this->GetReadAllTensors());
      preader->SetReadAllColorScalars(this->GetReadAllColorScalars());
      preader->SetReadAllTCoords(this->GetReadAllTCoords());
      preader->SetReadAllFields(this->GetReadAllFields());
      preader->Update();
      // Can we use the old output?
      if(!(output && strcmp(output->GetClassName(), "vtkStructuredPoints") == 0))
        {
        // Hack to make sure that the object is not modified
        // with SetNthOutput. Otherwise, extra executions occur.
        vtkTimeStamp ts = this->MTime;
        output = vtkStructuredPoints::New();
        this->GetExecutive()->SetOutputData(0, output);
        output->Delete();
        this->MTime = ts;
        }
      output->ShallowCopy(preader->GetOutput());
      output->GetPipelineInformation()->CopyEntry(
        preader->GetOutput()->GetPipelineInformation(),
        vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT());
      preader->Delete();
      return 1;
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
      preader->SetReadAllNormals(this->GetReadAllNormals());
      preader->SetReadAllTensors(this->GetReadAllTensors());
      preader->SetReadAllColorScalars(this->GetReadAllColorScalars());
      preader->SetReadAllTCoords(this->GetReadAllTCoords());
      preader->SetReadAllFields(this->GetReadAllFields());
      preader->Update();
      // Can we use the old output?
      if(!(output && output->IsA("vtkStructuredGrid")))
        {
        // Hack to make sure that the object is not modified
        // with SetNthOutput. Otherwise, extra executions occur.
        vtkTimeStamp ts = this->MTime;
        output = vtkStructuredGrid::New();
        this->GetExecutive()->SetOutputData(0, output);
        output->Delete();
        this->MTime = ts;
        }
      output->ShallowCopy(preader->GetOutput());
      preader->Delete();
      return 1;
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
      preader->SetReadAllNormals(this->GetReadAllNormals());
      preader->SetReadAllTensors(this->GetReadAllTensors());
      preader->SetReadAllColorScalars(this->GetReadAllColorScalars());
      preader->SetReadAllTCoords(this->GetReadAllTCoords());
      preader->SetReadAllFields(this->GetReadAllFields());
      preader->Update();
      // Can we use the old output?
      if(!(output && output->IsA("vtkRectilinearGrid")))
        {
        // Hack to make sure that the object is not modified
        // with SetNthOutput. Otherwise, extra executions occur.
        vtkTimeStamp ts = this->MTime;
        output = vtkRectilinearGrid::New();
        this->GetExecutive()->SetOutputData(0, output);
        output->Delete();
        this->MTime = ts;
        }
      output->ShallowCopy(preader->GetOutput());
      preader->Delete();
      return 1;
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
      preader->SetReadAllNormals(this->GetReadAllNormals());
      preader->SetReadAllTensors(this->GetReadAllTensors());
      preader->SetReadAllColorScalars(this->GetReadAllColorScalars());
      preader->SetReadAllTCoords(this->GetReadAllTCoords());
      preader->SetReadAllFields(this->GetReadAllFields());
      preader->Update();
      // Can we use the old output?
      if(!(output && strcmp(output->GetClassName(), "vtkUnstructuredGrid") == 0))
        {
        // Hack to make sure that the object is not modified
        // with SetNthOutput. Otherwise, extra executions occur.
        vtkTimeStamp ts = this->MTime;
        output = vtkUnstructuredGrid::New();
        this->GetExecutive()->SetOutputData(0, output);
        output->Delete();
        this->MTime = ts;
        }
      output->ShallowCopy(preader->GetOutput());
      preader->Delete();
      return 1;
      }
    default:
        vtkErrorMacro("Could not read file " << this->FileName);
    }
  return 0;
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
    vtkDebugMacro(<< "Premature EOF reading dataset keyword");
    return -1;
    }

  if ( !strncmp(this->LowerCase(line),"dataset",(unsigned long)7) )
    {
    // See if type is recognized.
    //
    if (!this->ReadString(line))
      {
      vtkDebugMacro(<< "Premature EOF reading type");
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
      vtkDebugMacro(<< "Cannot read dataset type: " << line);
      return -1;
      }
    }
  else if ( !strncmp(this->LowerCase(line),"field",(unsigned long)5) )
    {
    vtkDebugMacro(<<"This object can only read datasets, not fields");
    }
  else
    {
    vtkDebugMacro(<<"Expecting DATASET keyword, got " << line << " instead");
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

void vtkVisItDataSetReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

vtkDataSet *vtkVisItDataSetReader::GetOutput(int idx)
{
  return vtkDataSet::SafeDownCast(this->GetOutputDataObject(idx));
}

vtkDataSet *vtkVisItDataSetReader::GetOutput()
{
  return vtkDataSet::SafeDownCast(this->GetOutputDataObject(0));
}

int vtkVisItDataSetReader::FillOutputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkDataSet");
  return 1;
}

int vtkVisItDataSetReader::ProcessRequest(vtkInformation* request,
                                     vtkInformationVector** inputVector,
                                     vtkInformationVector* outputVector)
{
  // generate the data
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT()))
    {
    return this->RequestDataObject(request, inputVector, outputVector);
    }
  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}
