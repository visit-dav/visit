// .NAME vtkDataSetGradient

// .SECTION Credits
// Contributed by Thierry Carrard from
// Commissariat a l'Energie Atomique, (CEA)
// BP12, 91297 Arpajon, France

const static char * VTK_DATA_SET_GRADIENT__C_SCCS_ID = "%Z% DSSI/SNEC/LDDC %M%   %I%     %G%";

#include "vtkDataSetGradient.h"

#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkCell.h>
#include <vtkInformationVector.h>

#include <assert.h>

#include <map>
#include <set>
#include <algorithm>
#include <functional>
using namespace std;

//#define VERBOSE_DEBUG 1

#ifdef VERBOSE_DEBUG
#undef vtkDebugMacro
#define vtkDebugMacro(x) std::cout<<"DEBUG:"<<__FILE__<<":"<<__LINE__<<" " x; std::cout.flush()
#undef vtkErrorMacro
#define vtkErrorMacro(x) std::cout<<"ERROR:"<<__FILE__<<":"<<__LINE__<<" " x; std::cout.flush()
#endif

// standard constructors and factory
vtkCxxRevisionMacro(vtkDataSetGradient, "$Revision: 1.0 $");
vtkStandardNewMacro(vtkDataSetGradient);

/*!
The default constructor
\sa ~vtkDataSetGradient()
*/
vtkDataSetGradient::vtkDataSetGradient()
    : GradientArrayName(0)
{
    this->SetGradientArrayName("gradient");
}

/*!
The destrcutor
\sa vtkDataSetGradient()
*/
vtkDataSetGradient::~vtkDataSetGradient()
{
}

int vtkDataSetGradient::RequestData(vtkInformation *request,
                                    vtkInformationVector **inputVector,
                                    vtkInformationVector *outputVector)
{
    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    // get connected input & output
    vtkDataSet *_output = vtkDataSet::SafeDownCast( outInfo->Get(vtkDataObject::DATA_OBJECT()) );
    vtkDataSet* _input = vtkDataSet::SafeDownCast( inInfo->Get(vtkDataObject::DATA_OBJECT()) );

    if( _input==0 || _output==0 )
    {
        vtkErrorMacro(<<"Missing input or output \n");
        return 0;
    }

    // get array to compute gradient from
    // JSM: WAS:
    //vtkDataArray* inArray = this->GetInputArrayToProcess( 0, _input );
    // but that didn't compile with our VTK; changed to:
    vtkDataArray* inArray = this->GetInputArrayToProcess( 0, inputVector );
    if( inArray==0 )
    {
        inArray = _input->GetPointData()->GetScalars();
    }
    if( inArray==0 )
    {
        inArray = _input->GetCellData()->GetScalars();
    }

    if( inArray==0 )
    {
        vtkErrorMacro(<<"no  input array to process\n");
        return 0;
    }

    vtkDebugMacro(<<"Input array to process : "<<inArray->GetName()<<"\n");

    bool pointData;
    if( _input->GetCellData()->GetArray(inArray->GetName()) == inArray )
    {
        pointData = false;
        vtkDebugMacro(<<"cell data to point gradient\n");
    }
    else if( _input->GetPointData()->GetArray(inArray->GetName()) == inArray )
    {
        pointData = true;
        vtkDebugMacro(<<"point data to cell gradient\n");
    }
    else
    {
        vtkErrorMacro(<<"input array must be cell or point data\n");
        return 0;
    }

    vtkDataArray* cqsArray = _input->GetFieldData()->GetArray("CQS");

    if( cqsArray==0 )
    {
        vtkErrorMacro(<<"no CQS array. Add a vtkCQS filter to your pipeline.\n");
        return 0;
    }

    // we're just adding a scalar field
    _output->ShallowCopy( _input );

    vtkIdType nCells = _input->GetNumberOfCells();
    vtkIdType nPoints = _input->GetNumberOfPoints();

    vtkDoubleArray* gradientArray = vtkDoubleArray::New();
    gradientArray->SetName( this->GradientArrayName );
    gradientArray->SetNumberOfComponents(3);

    if( pointData ) // compute cell gradient from point data
    {
        gradientArray->SetNumberOfTuples( nCells );
        vtkIdType cellPoint = 0;
        for(vtkIdType i=0;i<nCells;i++)
        {
            vtkCell* cell = _input->GetCell(i);
            int np = cell->GetNumberOfPoints();
            double gradient[3] = {0,0,0};
            for(int p=0;p<np;p++)
            {
                double cqs[3];
                cqsArray->GetTuple( cellPoint++ , cqs );
                double scalar = inArray->GetTuple1( cell->GetPointId(p) );
                gradient[0] += scalar * cqs[0];
                gradient[1] += scalar * cqs[1];
                gradient[2] += scalar * cqs[2];
            }
            gradientArray->SetTuple( i , gradient );
        }
        _output->GetCellData()->AddArray( gradientArray );
        //_output->GetCellData()->SetVectors( gradientArray );
    }
    else // compute point gradient from cell data
    {
        gradientArray->SetNumberOfTuples( nPoints );
        gradientArray->FillComponent(0, 0.0);
        gradientArray->FillComponent(1, 0.0);
        gradientArray->FillComponent(2, 0.0);
        double * gradient = gradientArray->WritePointer(0,nPoints*3);
        vtkIdType cellPoint = 0;
        for(vtkIdType i=0;i<nCells;i++)
        {
            vtkCell* cell = _input->GetCell(i);
            int np = cell->GetNumberOfPoints();
            double scalar = inArray->GetTuple1( i );
            for(int p=0;p<np;p++)
            {
                double cqs[3];
                cqsArray->GetTuple( cellPoint++ , cqs );
                vtkIdType pointId = cell->GetPointId(p);
                gradient[pointId*3+0] += scalar * cqs[0];
                gradient[pointId*3+1] += scalar * cqs[1];
                gradient[pointId*3+2] += scalar * cqs[2];
            }
        }
        _output->GetPointData()->AddArray( gradientArray );
        //_output->GetPointData()->SetVectors( gradientArray );
    }
    gradientArray->Delete();

    vtkDebugMacro(<<_output->GetClassName()<<" @ "<<_output<<" :\n");
    // _output->PrintSelf(std::cout,4);

    return 1;
}

