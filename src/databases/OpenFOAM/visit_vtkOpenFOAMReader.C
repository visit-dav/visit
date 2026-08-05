// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "visit_vtkOpenFOAMReader.h"

#include <vtkAppendCompositeDataLeaves.h>
#include <vtkCharArray.h>
#include <vtkCollection.h>
#include <vtkDataArraySelection.h>
#include <vtkDirectory.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkIntArray.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkSortDataArray.h>
#include <vtkStdString.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStringArray.h>

vtkStandardNewMacro(visit_vtkOpenFOAMReader)

//-----------------------------------------------------------------------------
visit_vtkOpenFOAMReader::visit_vtkOpenFOAMReader()
{
    this->CaseType = RECONSTRUCTED_CASE;
    this->MTimeOld = 0;
}

//-----------------------------------------------------------------------------
visit_vtkOpenFOAMReader::~visit_vtkOpenFOAMReader()
{
}

//-----------------------------------------------------------------------------
void visit_vtkOpenFOAMReader::PrintSelf(ostream &os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
    os << indent << "Case Type: " << this->CaseType << endl;
    os << indent << "MTimeOld: " << this->MTimeOld << endl;
}

//-----------------------------------------------------------------------------
void visit_vtkOpenFOAMReader::SetCaseType(const int t)
{
    if (this->CaseType != t)
    {
        this->CaseType = static_cast<caseType>(t);
        this->Refresh = true;
        this->Modified();
    }
}

//-----------------------------------------------------------------------------
int visit_vtkOpenFOAMReader::RequestInformation(vtkInformation *request,
    vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    if (this->CaseType == RECONSTRUCTED_CASE)
    {
        return this->Superclass::RequestInformation(request, inputVector, outputVector);
    }

    if (!this->Superclass::FileName || strlen(this->Superclass::FileName) == 0)
    {
        vtkErrorMacro("FileName has to be specified!");
        return 0;
    }

    if (this->Superclass::FileNameOld != this->Superclass::FileName ||
        (this->Superclass::ListTimeStepsByControlDict !=
         this->Superclass::ListTimeStepsByControlDictOld) ||
        this->Superclass::Refresh)
    {
        // retain selection status when just refreshing a case
        if (!this->Superclass::FileNameOld.empty() &&
            this->Superclass::FileNameOld != this->Superclass::FileName)
        {
            // clear selections
            this->Superclass::CellDataArraySelection->RemoveAllArrays();
            this->Superclass::PointDataArraySelection->RemoveAllArrays();
            this->Superclass::LagrangianDataArraySelection->RemoveAllArrays();
            this->Superclass::PatchDataArraySelection->RemoveAllArrays();
        }

        this->Superclass::FileNameOld = this->FileName;
        this->Superclass::Readers.clear();

        vtkStringArray *procNames = vtkStringArray::New();

        // recreate case information
        vtkStdString masterCasePath, controlDictPath;
        this->Superclass::CreateCasePath(masterCasePath, controlDictPath);

        this->Superclass::CreateCharArrayFromString(this->Superclass::CasePath,
          "CasePath", masterCasePath);

        // search and list processor subdirectories
        vtkDirectory *dir = vtkDirectory::New();
        if (!dir->Open(masterCasePath.c_str()))
        {
            vtkErrorMacro(<< "Can't open " << masterCasePath.c_str());
            dir->Delete();
            return 0;
        }
        vtkIntArray *procNos = vtkIntArray::New();
        for (int fileI = 0; fileI < dir->GetNumberOfFiles(); fileI++)
        {
            const vtkStdString subDir(dir->GetFile(fileI));
            if (subDir.substr(0, 9) == "processor")
            {
                const vtkStdString procNoStr(subDir.substr(9, vtkStdString::npos));
                char *conversionEnd;
                const int procNo = strtol(procNoStr.c_str(), &conversionEnd, 10);
                if (procNoStr.c_str() + procNoStr.length() == conversionEnd && procNo >= 0)
                {
                    procNos->InsertNextValue(procNo);
                    procNames->InsertNextValue(subDir);
                }
            }
        }
        procNos->Squeeze();
        procNames->Squeeze();
        dir->Delete();

        // sort processor subdirectories by processor numbers
        vtkSortDataArray::Sort(procNos, procNames);
        procNos->Delete();

        // get time directories from the first processor subdirectory
        if (procNames->GetNumberOfTuples() > 0)
        {
            vtkOpenFOAMReader *masterReader = vtkOpenFOAMReader::New();
            masterReader->SetFileName(this->FileName);
            masterReader->SetParent(this);
            if (!masterReader->MakeInformationVector(outputVector, procNames ->GetValue(0)) || !masterReader->MakeMetaDataAtTimeStep(true))
            {
                procNames->Delete();
                masterReader->Delete();
                return 0;
            }
            this->Superclass::Readers.push_back(masterReader);
            masterReader->Delete();

            // create reader instances for other processor subdirectories
            // skip processor0 since it's already created
            for (int i = 0; i < procNames->GetNumberOfTuples(); ++i)
            {
                vtkOpenFOAMReader *subReader = vtkOpenFOAMReader::New();
                subReader->SetFileName(this->FileName);
                subReader->SetParent(this);
                // if getting metadata failed simply delete the reader instance
                if (subReader->MakeInformationVector(NULL, procNames->GetValue(i))
                    && subReader->MakeMetaDataAtTimeStep(true))
                {
                    this->Superclass::Readers.push_back(subReader);
                }
                else
                {
                    vtkWarningMacro(<<"Removing reader for processor subdirectory "
                        << procNames->GetValue(i).c_str());
                }
                subReader->Delete();
            }
        }

        procNames->Delete();

        this->Superclass::Refresh = false;
    }

    return 1;
}

//-----------------------------------------------------------------------------
int visit_vtkOpenFOAMReader::RequestData(vtkInformation *request,
    vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
    if (this->CaseType == RECONSTRUCTED_CASE)
    {
        return this->Superclass::RequestData(request, inputVector, outputVector);
    }

    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkMultiBlockDataSet *output =
          vtkMultiBlockDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

    int ret = 1;
    if (!this->Superclass::Readers.empty())
    {
        int nSteps = 0;
        double requestedTimeValue(0);
        if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
        {
            requestedTimeValue = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
            nSteps = outInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
            if (nSteps > 0)
            {
                outInfo->Set(vtkDataObject::DATA_TIME_STEP(), requestedTimeValue);
            }
        }

        vtkAppendCompositeDataLeaves *append = vtkAppendCompositeDataLeaves::New();
        // append->AppendFieldDataOn();

        vtkOpenFOAMReader *reader;
        for (size_t i = 0; i < this->Superclass::Readers.size(); ++i)
        {
            reader = vtkOpenFOAMReader::SafeDownCast(this->Superclass::Readers[i]);
            if (reader != NULL)
            {
                // even if the child readers themselves are not modified, mark
                // them as modified if "this" has been modified, since they
                //   refer to the property of "this"
                if ((nSteps > 0 && reader->SetTimeValue(requestedTimeValue)) ||
                     this->MTimeOld != this->GetMTime())
                {
                    reader->Modified();
                }
                if (reader->MakeMetaDataAtTimeStep(false))
                {
                    append->AddInputConnection(reader->GetOutputPort());
                }
            }
        }

        if (append->GetNumberOfInputConnections(0) == 0)
        {
            output->Initialize();
            ret = 0;
        }
        else
        {
            // reader->RequestInformation() and RequestData() are called
            // for all reader instances without setting UPDATE_TIME_STEPS
            append->Update();
            output->ShallowCopy(append->GetOutput());
        }
        append->Delete();

        // known issue: output for process without sub-reader will not have CasePath
        output->GetFieldData()->AddArray(this->Superclass::CasePath);
    }
    else
    {
        output->Initialize();
    }

    this->Superclass::UpdateStatus();
    this->MTimeOld = this->GetMTime();

    return ret;
}

int
visit_vtkOpenFOAMReader::GetCellArrayExists(const char *name)
{
    return this->CellDataArraySelection->ArrayExists(name);
}

int
visit_vtkOpenFOAMReader::GetPointArrayExists(const char *name)
{
    return this->PointDataArraySelection->ArrayExists(name);
}

int
visit_vtkOpenFOAMReader::GetLagrangianArrayExists(const char *name)
{
    return this->LagrangianDataArraySelection->ArrayExists(name);
}
