/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                             avtCentroidQuery.C                            //
// ************************************************************************* //

#include <avtCentroidQuery.h>

#include <vtkCell.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkVisItUtility.h>

#include <avtAbsValExpression.h>
#include <avtBinaryMultiplyExpression.h>
#include <avtCallback.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtVMetricArea.h>
#include <avtVMetricVolume.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <InvalidDimensionsException.h>
#include <NonQueryableInputException.h>

#include <snprintf.h>
#include <float.h>
#include <math.h>

using     std::string;
using     std::vector;

#if !defined(M_PI)
#define M_PI 3.14159265358979323846
#endif

// ****************************************************************************
//  Method: avtCentroidQuery constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Aug 12 15:25:07 PDT 2005
//    Add absval.
//
// ****************************************************************************

avtCentroidQuery::avtCentroidQuery()
{
    absval = new avtAbsValExpression;
    multiply = new avtBinaryMultiplyExpression;
    area = new avtVMetricArea;
    volume = new avtVMetricVolume;
}


// ****************************************************************************
//  Method: avtCentroidQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Aug 12 15:25:07 PDT 2005
//    Destruct absval.
//
// ****************************************************************************

avtCentroidQuery::~avtCentroidQuery()
{
    delete absval;
    delete multiply;
    delete volume;
    delete area;
}


// ****************************************************************************
//  Method: avtCentroidQuery::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
// ****************************************************************************

void
avtCentroidQuery::PreExecute(void)
{
    avtDatasetQuery::PreExecute();

    for (int i = 0 ; i < 3 ; i++)
        C[i] = 0.;
    total_mass = 0.;
}

// ****************************************************************************
//  Method: avtCentroidQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
//  Modifications:
//
//    Cyrus Harrison, Tue Sep 18 13:45:35 PDT 2007
//    Added support for user settable floating point format string
//
// ****************************************************************************

void
avtCentroidQuery::PostExecute(void)
{
    double C_tmp[3];
    SumDoubleAcrossAllProcessors(total_mass);
    SumDoubleArrayAcrossAllProcessors(C, C_tmp, 3);
    if (total_mass != 0.)
    {
        C_tmp[0] /= total_mass;
        C_tmp[1] /= total_mass;
        C_tmp[2] /= total_mass;
    }

    //
    //  Parent class uses this message to set the Results message
    //  in the Query Attributes that is sent back to the viewer.
    //  That is all that is required of this query.
    //
    char msg[4096];
    string floatFormat = queryAtts.GetFloatFormat();
    string format = "Centroid = (" + floatFormat + ", "
                                   + floatFormat + ", "
                                   + floatFormat + ")";
    SNPRINTF(msg, 4096, format.c_str(),C_tmp[0],C_tmp[1],C_tmp[2]);
    SetResultMessage(msg);
    doubleVector C_vec(3);
    for (int i = 0 ; i < 3 ; i++)
        C_vec[i] = C_tmp[i];
    SetResultValues(C_vec);
}

// ****************************************************************************
//  Method: avtCentroidQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
// ****************************************************************************

void
avtCentroidQuery::Execute(vtkDataSet *ds, const int dom)
{
    int nCells = ds->GetNumberOfCells();
    vtkDataArray *ghosts = ds->GetCellData()->GetArray("avtGhostZones");
    vtkDataArray *var = ds->GetCellData()->GetArray("avt_mass");
    if (var == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }
    for (int i = 0 ; i < nCells ; i++)
    {
        if (ghosts != NULL && ghosts->GetTuple1(i) != 0.)
            continue;
        vtkCell *cell = ds->GetCell(i);
        double center[3];
        vtkVisItUtility::GetCellCenter(cell, center);
        double mass = var->GetTuple1(i);
        C[0] += mass*center[0];
        C[1] += mass*center[1];
        C[2] += mass*center[2];
        total_mass += (mass < 0. ? -mass : mass);
    }
}


// ****************************************************************************
//  Method: avtCentroidQuery::ApplyFilters
//
//  Purpose:
//      Applies the filters to the input.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Aug 12 15:25:07 PDT 2005
//    Use absolute value of the volume or area.
//
//    Hank Childs, Wed Sep 14 16:01:12 PDT 2005
//    Use topological dimension for weights, not spatial dimension.
//
//    Kathleen Bonnell, Wed Apr  2 12:01:59 PDT 2008 
//    Retrieve the varname from the queryAtts instead of DataRequest, as
//    DataRequest may have the wrong value based on other pipelines sharing
//    the same source. 
//
// ****************************************************************************

avtDataObject_p
avtCentroidQuery::ApplyFilters(avtDataObject_p inData)
{
    //
    // Create an artificial pipeline.
    //
    avtDataset_p ds;
    CopyTo(ds, inData);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p dob = termsrc.GetOutput();

    //
    // Set up our base class so it is ready to sum.
    //
    string varname = queryAtts.GetVariables()[0];
    bool useVar = false;
    if (GetInput()->GetInfo().GetAttributes().ValidVariable(varname))
    {
        int dim = GetInput()->GetInfo().GetAttributes().
                                         GetVariableDimension(varname.c_str());
        if (dim == 1)
            useVar = true;
    }

    avtVerdictExpression *vf = NULL;
    if (dob->GetInfo().GetAttributes().GetTopologicalDimension() == 3)
        vf = volume;
    else
        vf = area;
    vf->SetOutputVariableName("avt_verdict");
    vf->SetInput(dob);
 
    absval->SetInput(vf->GetOutput());
    absval->AddInputVariableName("avt_verdict");
    if (useVar)
        absval->SetOutputVariableName("avt_weights");
    else
        absval->SetOutputVariableName("avt_mass");
    dob = absval->GetOutput();

    if (useVar)
    {
        multiply->SetInput(dob);
        multiply->ClearInputVariableNames();
        multiply->AddInputVariableName(varname.c_str());
        multiply->AddInputVariableName("avt_weights");
        multiply->SetOutputVariableName("avt_mass");
        dob = multiply->GetOutput();
    }

    //
    // Cause our artificial pipeline to execute.
    //
    avtContract_p contract =
        inData->GetOriginatingSource()->GetGeneralContract();
    dob->Update(contract);
    return dob;
}


