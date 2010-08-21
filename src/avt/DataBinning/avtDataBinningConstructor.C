/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                         avtDataBinningConstructor.C                       //
// ************************************************************************* //

#include <avtDataBinningConstructor.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <ConstructDataBinningAttributes.h>

#include <avtCallback.h>
#include <avtDataBinning.h>
#include <avtDataBinningFunctionInfo.h>
#include <avtParallel.h>
#include <avtR2Faverage.h>
#include <avtR2Fminimum.h>
#include <avtR2Fmaximum.h>
#include <avtR2Fcount.h>
#include <avtR2Fpdf.h>
#include <avtR2Fstddev.h>
#include <avtR2Fsum.h>
#include <avtR2Fvariance.h>
#include <avtR2Frms.h>
#include <avtUniformBinningScheme.h>

#include <snprintf.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ExpressionException.h>
#include <PipelineException.h>
#include <ImproperUseException.h>

using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtDataBinningConstructor constructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtDataBinningConstructor::avtDataBinningConstructor()
{
    ;
}


// ****************************************************************************
//  Method: avtDataBinningConstructor destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtDataBinningConstructor::~avtDataBinningConstructor()
{
    ;
}


// ****************************************************************************
//  Method: avtDataBinningConstructor::ConstructDataBinning
//
//  Purpose:
//      Constructs the actual data binning.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Feb 25 15:01:56 PST 2006
//    Add support for time.
//
//    Hank Childs, Mon May 15 14:25:28 PDT 2006
//    Fix memory leak.
//
//    Hank Childs, Fri Jun  9 13:44:39 PDT 2006
//    Fix problem with mixed centering. Also add "default" to switch statement.
//
//    Hank Childs, Mon Aug 20 10:07:01 PDT 2007
//    Detect and report errors that happen upstream when getting the data ready
//    to calculate the DDF.
//
//    Hank Childs, Mon Dec 10 16:54:53 PST 2007
//    Make sure, for unstructured grids, we are only using nodes that are
//    referenced by cells.
//
//    Sean Ahern, Thu Jan 10 16:01:06 EST 2008
//    Added a "count" statistical operator.
//
//    Cyrus Harrison, Tue Aug 10 10:48:17 PDT 2010
//    Added a "rms" operator.
//
//    Hank Childs, Sat Aug 21 14:05:14 PDT 2010
//    Better support for mixed centering, r2f's that don't require a variable.
//
// ****************************************************************************

avtDataBinning *
avtDataBinningConstructor::ConstructDataBinning(
                                         ConstructDataBinningAttributes *atts,
                                         avtContract_p spec, bool mustReExecute)
{
    int   i, j, k, l;

    //
    // Certify the attributes.
    //
    if (atts->GetBinningScheme() != ConstructDataBinningAttributes::Uniform)
    {
        debug1 << "Only uniform binning is supported" << endl;
        EXCEPTION1(ImproperUseException, "Only uniform binning is supported");
        return NULL;
    }
    if (atts->ReductionRequiresVariable() && atts->GetVarForReductionOperator() == "")
    {
        debug1 << "The reduction operator requires a variable." << endl;
        EXCEPTION1(ImproperUseException, "The reduction operator requires a variable.");
        return NULL;
    }
    for (i = 0 ; i < atts->GetVarnames().size() ; i++)
    {
        if (atts->GetVarnames()[i] == "")
        {
            debug1 << "Some variable names are not valid." << endl;
            EXCEPTION1(ImproperUseException, 
                       "Some variable names are not valid.");
            return NULL;
        }
    }
    if (atts->GetNumBins().size() != atts->GetVarnames().size())
    {
        debug1 << "There must be the same number of samples "
                                  "as there are variable names." << endl;
        EXCEPTION1(ImproperUseException,
                   "There must be the same number of samples "
                   "as there are variable names.");
        return NULL;
    }
    if (atts->GetBinBoundaries().size() != 2*atts->GetVarnames().size())
    {
        debug1 << "There must be a minimum and a maximum for "
                  "every variable.  The range is the wrong size." << endl;
        EXCEPTION1(ImproperUseException,
                   "There must be a minimum and a maximum for "
                   "every variable.  The range is the wrong size.");
        return NULL;
    }
    for (i = 0 ; i < atts->GetVarnames().size() ; i++)
    {
        if (atts->GetBinBoundaries()[2*i] >= atts->GetBinBoundaries()[2*i+1])
        {
            debug1 << "The minimum for the range of a variable"
                                      "must be less than the maximum" << endl;
            EXCEPTION1(ImproperUseException,
                       "The minimum for the range of a variable"
                       "must be less than the maximum");
            return NULL;
        }
    }
    for (i = 0 ; i < atts->GetVarnames().size() ; i++)
    {
        if (atts->GetNumBins()[i] <= 0)
        {
            debug1 << "Each variable must have at least one bin" << endl;
            EXCEPTION1(ImproperUseException,
                       "Each variable must have at least one bin");
            return NULL;
        }
    }
   

    //
    // Set up the binning scheme and the function info based on the input
    // arguments.
    //
    int nvars = atts->GetVarnames().size();
    float *minmax = new float[2*nvars];
    for (i = 0 ; i < 2*nvars ; i++)
        minmax[i] = atts->GetBinBoundaries()[i];
    int *nvals = new int[nvars];
    for (i = 0 ; i < nvars ; i++)
        nvals[i] = atts->GetNumBins()[i];
    avtBinningScheme *bs = new avtUniformBinningScheme(nvars, minmax, nvals);
    std::vector<std::string> varnames = atts->GetVarnames();
    avtDataBinningFunctionInfo *info = new avtDataBinningFunctionInfo(bs, varnames,
                                                      atts->GetVarForReductionOperator());

    //
    // Now create the "relation-to-function operator".
    //
    int nBins = bs->GetNumberOfBins();
    bs->SetOutOfBoundsBehavior(atts->GetOutOfBoundsBehavior());
    avtR2Foperator *R2Foperator = NULL;
    switch (atts->GetReductionOperator())
    {
      case ConstructDataBinningAttributes::Average:
        R2Foperator = new avtR2Faverage(nBins, atts->GetUndefinedValue());
        break;
      case ConstructDataBinningAttributes::Minimum:
        R2Foperator = new avtR2Fminimum(nBins, atts->GetUndefinedValue());
        break;
      case ConstructDataBinningAttributes::Maximum:
        R2Foperator = new avtR2Fmaximum(nBins, atts->GetUndefinedValue());
        break;
      case ConstructDataBinningAttributes::Count:
        R2Foperator = new avtR2Fcount(nBins);
        break;
      case ConstructDataBinningAttributes::PDF:
        R2Foperator = new avtR2Fpdf(nBins);
        break;
      case ConstructDataBinningAttributes::StandardDeviation:
        R2Foperator = new avtR2Fstddev(nBins, atts->GetUndefinedValue());
        break;
      case ConstructDataBinningAttributes::Sum:
        R2Foperator = new avtR2Fsum(nBins);
        break;
      case ConstructDataBinningAttributes::Variance:
        R2Foperator = new avtR2Fvariance(nBins, atts->GetUndefinedValue());
        break;
      case ConstructDataBinningAttributes::RMS:
        R2Foperator = new avtR2Frms(nBins, atts->GetUndefinedValue());
        break;
      default:
        // Caught in logic below
        break;
    }
    if (R2Foperator == NULL)
    {
        EXCEPTION2(ExpressionException, atts->GetName(),
                   "You have requested an operator that has not been implemented yet.");
    }

    //
    // Make the input pipeline execute again to get the variables we need.
    //
    int timeStart = spec->GetDataRequest()->GetTimestep();
    int timeEnd = timeStart+1;
    int timeStride = 1;
    if (atts->GetOverTime())
    {
        timeStart  = atts->GetTimeStart();
        timeEnd    = atts->GetTimeEnd();
        timeStride = atts->GetTimeStride();
    }

    //
    // Now iterate some number of passes over the data.  This will probably
    // just be one pass, but could be multiple passes in the case of operators
    // like "standard deviation".  On the final pass, the operator will return
    // the actual data.  On previous passes, it will return NULL.
    //
    int nPasses = R2Foperator->GetNumberOfPasses();
    float *vals = NULL;
    for (int pass = 0 ; pass < nPasses ; pass++)
    {
        for (int time = timeStart ; time < timeEnd ; time += timeStride)
        {
            avtContract_p spec2 = new avtContract(spec);
            avtDataRequest_p dataRequest = spec2->GetDataRequest();
            if (atts->GetVarForReductionOperator() != "")
                dataRequest->AddSecondaryVariable(atts->GetVarForReductionOperator().c_str());
            for (i = 0 ; i < atts->GetVarnames().size() ; i++)
                dataRequest->AddSecondaryVariable(atts->GetVarnames()[i].c_str());
            dataRequest->SetTimestep(time);
            if (mustReExecute)
                GetInput()->Update(spec2);
            if (GetInput()->GetInfo().GetValidity().HasErrorOccurred())
            {
                char msg[1024];
                SNPRINTF(msg, 1024, "The data binning could not be calculated because "
                                    "an error occurred when generating "
                                    "the data to construct it from.  The error"
                                    " was \"%s\".",
                GetInput()->GetInfo().GetValidity().GetErrorMessage().c_str());
                EXCEPTION1(ImproperUseException, msg);
            }

            //
            // Get all of the input data.
            //
            avtDataTree_p tree = GetInputDataTree();
            int nLeaves;
            vtkDataSet **leaves = tree->GetAllLeaves(nLeaves);
        
            bool coDomIsNodal = true;
            bool *isNodal = new bool[nvars];
            const char   *codomain_varname = info->GetCodomainName().c_str();
            bool hasError = false;
            bool mixedCentering = false;
            if (nLeaves > 0)
            {
                for (k = 0 ; k < nvars ; k++)
                {
                    const char *varname = info->GetDomainTupleName(k).c_str();
                    if (leaves[0]->GetPointData()->GetArray(varname) != NULL)
                        isNodal[k] = true;
                    else if (leaves[0]->GetCellData()->GetArray(varname) 
                                                                       != NULL)
                        isNodal[k] = false;
                    else
                        hasError = true;
                }
    
                if (atts->ReductionRequiresVariable())
                {
                    if (leaves[0]->GetPointData()->GetArray(codomain_varname)
                                                                       != NULL)
                        coDomIsNodal = true;
                    else if (leaves[0]->GetCellData()->GetArray(codomain_varname) 
                                                                       != NULL)
                        coDomIsNodal = false;
                    else
                        hasError = true;

                    for (k = 0 ; k < nvars ; k++)
                        if (isNodal[k] != coDomIsNodal)
                            mixedCentering = true;
                }
            }

            int val = UnifyMaximumValue((hasError ? 1 : 0));
            if (val > 0)
            {
                debug1 << "Could not create data binning because either a "
                                  "variable could not be located, or because"
                                  " the centering of the variables do not "
                                  "match." << endl;
                EXCEPTION1(ImproperUseException, 
                                  "Could not create data binning because either a "
                                  "variable could not be located, or because"
                                  " the centering of the variables do not "
                                  "match.");
                return NULL;
            }
        
            vtkDataArray **arr = new vtkDataArray*[nvars];
            float        *args = new float[nvars];
            for (j = 0 ; j < nLeaves ; j++)
            {
                vtkDataArray *codomain = (coDomIsNodal 
                        ? leaves[j]->GetPointData()->GetArray(codomain_varname)
                        : leaves[j]->GetCellData()->GetArray(codomain_varname));
                for (k = 0 ; k < nvars ; k++)
                {
                    const char *varname = info->GetDomainTupleName(k).c_str();
                    arr[k] = (isNodal[k]
                                ? leaves[j]->GetPointData()->GetArray(varname)
                                : leaves[j]->GetCellData()->GetArray(varname));
                }
                int nvals;
                bool doCells = true;
                if (mixedCentering)
                    doCells = true;
                else if (atts->ReductionRequiresVariable())
                {
                    if (!coDomIsNodal)
                        doCells = true;
                    else
                        doCells = false;
                }
                else
                {
                    if (! isNodal[0])
                        doCells = true;
                    else
                        doCells = false;
                }

                nvals = (doCells ? leaves[j]->GetNumberOfCells()
                                 : leaves[j]->GetNumberOfPoints());

                vector<bool> useValue;
                if (!doCells)
                {
                    if (leaves[j]->GetDataObjectType() == VTK_UNSTRUCTURED_GRID ||
                        leaves[j]->GetDataObjectType() == VTK_POLY_DATA)
                    {
                        useValue.resize(nvals, false);
                        int ncells = leaves[j]->GetNumberOfCells();
                        for (l = 0 ; l < ncells ; l++)
                        {
                            vtkCell *cell = leaves[j]->GetCell(l);
                            for (int p = 0 ; p < cell->GetNumberOfPoints() ; p++)
                            {
                                vtkIdType ptId = cell->GetPointId(p);
                                useValue[ptId] = true;
                            }
                        }
                    }
                }

                for (l = 0 ; l < nvals ; l++)
                {
                    if (useValue.size() > 0)
                        if (!useValue[l])
                            continue;

                    if (!mixedCentering)
                    {
                        for (k = 0 ; k < nvars ; k++)
                            args[k] = arr[k]->GetTuple1(l);
                        int binId = bs->GetBinId(args);
                        float cval = 0.;
                        if (atts->ReductionRequiresVariable())
                            cval = codomain->GetTuple1(l);
                        if (binId >= 0)
                            R2Foperator->AddData(binId, cval);
                    }
                    else
                    {
                        vtkCell *cell = leaves[j]->GetCell(l);
                        for (int p = 0 ; p < cell->GetNumberOfPoints() ; p++)
                        {
                            vtkIdType ptId = cell->GetPointId(p);
                            for (k = 0 ; k < nvars ; k++)
                                if (isNodal[k])
                                    args[k] = arr[k]->GetTuple1(ptId);
                                else
                                    args[k] = arr[k]->GetTuple1(l);
                            int binId = bs->GetBinId(args);
                            float cval = 0;
                            if (atts->ReductionRequiresVariable())
                                 cval =  (coDomIsNodal
                                                 ? codomain->GetTuple1(ptId)
                                                 : codomain->GetTuple1(l));
                            if (binId >= 0)
                                R2Foperator->AddData(binId,cval);
                        }
                    }
                }
            }
            delete [] arr;
            delete [] args;
            delete [] leaves;
            delete [] isNodal;
        }

        vals = R2Foperator->FinalizePass(pass);
    }

    //
    // We can now construct the actual data binning.
    //
    avtDataBinning *rv = new avtDataBinning(info, vals);

    //
    // Clean up memory.
    //
    delete R2Foperator;
    delete [] minmax;
    delete [] nvals;
    // Don't delete binning scheme (bs) function info (info), or "vals"
    // since the data binning now owns them.

    return rv;
}


