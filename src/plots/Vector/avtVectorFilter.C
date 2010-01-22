/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                             avtVectorFilter.C                             //
// ************************************************************************* //

#include <avtVectorFilter.h>

#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkVectorReduceFilter.h>
#include <vtkVertexFilter.h>
#include <avtParallel.h>

#include <Expression.h>
#include <ExpressionList.h>
#include <ParsingExprList.h>

#include <string.h>
using std::string;


// ****************************************************************************
//  Method: avtVectorFilter constructor
//
//  Arguments:
//      us      A boolean indicating if you should use the stride (true) or
//              stick with a fixed number of vectors (false).
//      red     The reduction factor, the stride of number of vectors depending
//              on us.
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Aug 30 17:30:48 PDT 2001
//    Added vertex filter.
//
//    Kathleen Bonnell, Tue Oct 12 16:18:37 PDT 2004 
//    Added keepNodeZone. 
//
//    Hank Childs, Fri Mar 11 15:01:22 PST 2005
//    Instantiate VTK filters on the fly.  Makes memory issues easier.
//
//    Jeremy Meredith, Tue Jul  8 11:11:59 EDT 2008
//    Added ability to limit vectors to come from original cell only
//    (useful for material-selected vector plots).
//
//    Jeremy Meredith, Mon Jul 14 12:40:41 EDT 2008
//    Keep track of the approximate number of domains to be plotted.
//    This will let us calculate a much closer stride value if the
//    user requests a particular number of vectors to be plotted.
//
// ****************************************************************************

avtVectorFilter::avtVectorFilter(bool us, int red)
{
    if (us)
    {
        SetStride(red);
    }
    else
    {
        SetNVectors(red);
    }

    keepNodeZone = false;
    origOnly = false;
    approxDomains = 1;
}


// ****************************************************************************
//  Method: avtVectorFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   March 21, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Aug 30 17:30:48 PDT 2001
//    Added vertex filter.
//
//    Hank Childs, Fri Mar 11 15:01:22 PST 2005
//    Instantiate VTK filters on the fly.  Makes memory issues easier.
//
// ****************************************************************************

avtVectorFilter::~avtVectorFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtVectorFilter::SetStride
//
//  Purpose:
//      Sets the stride of reduction for the vector.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Mar 11 15:01:22 PST 2005
//    Instantiate VTK filters on the fly.  Makes memory issues easier.
//
// ****************************************************************************

void
avtVectorFilter::SetStride(int s)
{
    useStride = true;
    stride    = s;
    nVectors  = -1;
}


// ****************************************************************************
//  Method: avtVectorFilter::SetNVectors
//
//  Purpose:
//      Sets the number of vectors the filter should try to output.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Mar 11 15:01:22 PST 2005
//    Instantiate VTK filters on the fly.  Makes memory issues easier.
//
// ****************************************************************************

void
avtVectorFilter::SetNVectors(int n)
{
    useStride = false;
    stride    = -1;
    nVectors  = n;
}


// ****************************************************************************
//  Method:  avtVectorFilter::SetLimitToOriginal
//
//  Purpose:
//    when set to true, this will only draw one vector per original
//    cell/node.
//
//  Arguments:
//    orig       true to enable this reduction
//
//  Programmer:  Jeremy Meredith
//  Creation:    July  8, 2008
//
// ****************************************************************************

void
avtVectorFilter::SetLimitToOriginal(bool orig)
{
    origOnly = orig;
}

// ****************************************************************************
//  Method:  avtVectorFilter::PreExecute
//
//  Purpose:
//    Executes before the main execute loop.  In this case, we
//    just want to get a rough count of the number of domains
//    so that we can divide the requested number of vectors
//    by the number of domains.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 14, 2008
//
// ****************************************************************************

void
avtVectorFilter::PreExecute()
{
    avtDataTreeIterator::PreExecute();

    // Just in case there's something fishy about this technique,
    // skip the logic when we don't need it (i.e. if the stride was
    // specified directly).
    approxDomains = 1;
    if (!useStride)
    {
        approxDomains = GetInputDataTree()->GetNumberOfLeaves();
        SumIntAcrossAllProcessors(approxDomains);
        if (approxDomains < 1)
            approxDomains = 1;
    }
}

// ****************************************************************************
//  Method: avtVectorFilter::Equivalent
//
//  Purpose:
//      Determines if this vector filter is equivalent to the attributes.
//
//  Arguments:
//      us      A boolean indicating if you should use the stride (true) or
//              stick with a fixed number of vectors (false).
//      red     The reduction factor, the stride of number of vectors depending
//              on us.
//
//  Returns:     true if it is equal, false otherwise.
//
//  Programmer:  Hank Childs
//  Creation:    March 21, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Jul  8 11:12:38 EDT 2008
//    Added ability to limit vectors to come from original cell only
//    (useful for material-selected vector plots).
//
// ****************************************************************************

bool
avtVectorFilter::Equivalent(bool us, int red, bool orig)
{
    if (us != useStride)
    {
        return false;
    }
    if (useStride)
    {
        if (red != stride)
        {
            return false;
        }
    }
    else
    {
        if (red != nVectors)
        {
            return false;
        }
    }

    if (orig != origOnly)
        return false;

    return true;
}


// ****************************************************************************
//  Method: avtVectorFilter::ExecuteData
//
//  Purpose:
//      Takes in an input dataset and creates the vector poly data.
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer:   Hank Childs
//  Creation:     March 21, 2001
//
//  Modifications:
//    
//    Kathleen Bonnell, Tue Apr 10 11:51:18 PDT 2001
//    Renamed method as ExecuteData from ExecuteDomain.
//
//    Hank Childs, Thu Aug 30 17:30:48 PDT 2001
//    Added vertex filter.
//
//    Hank Childs, Wed Sep 11 08:53:50 PDT 2002
//    Fixed memory leak.
//
//    Hank Childs, Fri Mar 11 15:01:22 PST 2005
//    Instantiate VTK filters on the fly.  Makes memory issues easier.
//
//    Jeremy Meredith, Tue Jul  8 15:16:05 EDT 2008
//    Added ability to limit vectors to come from original cell only
//    (useful for material-selected vector plots).
//
//    Jeremy Meredith, Mon Jul 14 12:39:40 EDT 2008
//    Divide the requested number of vectors by the calculated number
//    of domains.  It's not perfect, but it should hopefully get us
//    close to the requested number.
//
//    Hank Childs, Thu Jan 21 20:18:42 PST 2010
//    Fixed a bug where we got no glyphs when there were more domains than
//    target glyphs.
//
// ****************************************************************************

vtkDataSet *
avtVectorFilter::ExecuteData(vtkDataSet *inDS, int, string)
{
    vtkVectorReduceFilter *reduce = vtkVectorReduceFilter::New();
    vtkVertexFilter       *vertex = vtkVertexFilter::New();
    if (useStride)
        reduce->SetStride(stride);
    else
    {
        int nPerDomain = nVectors / approxDomains;
        if (nPerDomain < 1)
            nPerDomain = 1;
        reduce->SetNumberOfElements(nPerDomain);
    }

    if (inDS->GetPointData()->GetVectors() != NULL)
    {
        vertex->VertexAtPointsOn();
    }
    else
    {
        vertex->VertexAtPointsOff();
    }
    reduce->SetLimitToOriginal(origOnly);

    vertex->SetInput(inDS);
    reduce->SetInput(vertex->GetOutput());

    vtkPolyData *outPD = reduce->GetOutput();
    outPD->Update();
    ManageMemory(outPD);
    vertex->Delete();
    reduce->Delete();
    return outPD;
}


// ****************************************************************************
//  Method: avtVectorFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Indicate that the vector are of dimension 0.
//
//  Programmer: Hank Childs
//  Creation:   June 12, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep  4 16:14:49 PDT 2001
//    Reflect new interface for avtDataAttributes.
//
//    Hank Childs, Wed Apr  9 13:53:09 PDT 2003
//    Do not calculate normals of these points.
//
//    Kathleen Bonnell, Tue Oct 12 16:02:33 PDT 2004 
//    Set whether or not we want to specify that node and zone arrays be
//    kept around through to the renderer.  (Needed by pick).
//
// ****************************************************************************

void
avtVectorFilter::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(0);
    GetOutput()->GetInfo().GetValidity().SetNormalsAreInappropriate(true);
    GetOutput()->GetInfo().GetAttributes().SetKeepNodeZoneArrays(keepNodeZone);
}


// ****************************************************************************
//  Method: avtVectorFilter::SetMagVarName
//
//  Purpose:
//    Sets the name that should be used when creating the magnitude data
//    array. 
//
//  Arguments:
//    mname     The name of the magnitude array.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 9, 2004 
//
// ****************************************************************************

void
avtVectorFilter::SetMagVarName(const string &mname)
{
    magVarName = mname;
}


// ****************************************************************************
//  Method: avtVectorFilter::ModifyContract
//
//  Purpose:  Create an expression for the magnitude of the requested
//            vector variable, so that the vectors are colored correctly.
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   August 9, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct 12 16:02:33 PDT 2004 
//    Store whether or not we want to specify that node and zone arrays be
//    kept around through to the renderer.  (Needed by pick).
//
//    Kathleen Bonnell, Wed Dec  1 09:19:31 PST 2004 
//    Turn on Node/Zone numbers when 'MayRequireNodes' is true. 
//
//    Kathleen Bonnell, Fri Jun 10 13:37:09 PDT 2005
//    Verify the existence of a valid variable before attempting to retrieve
//    its centering.
//
//    Kathleen Bonnell, Thu Jan 19 08:29:59 PST 2006
//    Surround the variable with <> in defining the magnitude expression,
//    to account for variables stored in subdirs.
// 
//    Jeremy Meredith, Tue Jul  8 11:10:58 EDT 2008
//    If we're asked to limit the vectors to one per original node/zone,
//    also add the original node/zone arrays.
//
//    Jeremy Meredith, Tue Jul 15 10:47:51 EDT 2008
//    Disable streaming if we need to calculate a stride based on the
//    number of requested vectors and the number of domains.
//
// ****************************************************************************

avtContract_p
avtVectorFilter::ModifyContract(avtContract_p contract)
{
    avtContract_p rv = contract;
    avtDataRequest_p ds = contract->GetDataRequest();

    //
    // Create the expression definition
    //
    string edef = string("magnitude(<") + ds->GetVariable() + string(">)");


    ExpressionList *elist = ParsingExprList::Instance()->GetList();
    Expression *e = new Expression();
        
    e->SetName(magVarName.c_str());
    e->SetDefinition(edef.c_str());
    e->SetType(Expression::ScalarMeshVar);
    elist->AddExpressions(*e);
    delete e;

    // Create a new dcontract so that we can add the secondary var.
    avtDataRequest_p nds = new avtDataRequest(ds->GetVariable(),
                ds->GetTimestep(), ds->GetRestriction());
    nds->AddSecondaryVariable(magVarName.c_str());
    rv = new avtContract(contract, nds);
    
    // If we're not using the stride, then we have to calculate
    // the per-domain vectorc count by dividing by the number of
    // domains, which we can't calculate if we're streaming.
    if (!useStride)
        rv->NoStreaming();

    avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();
    if (contract->GetDataRequest()->MayRequireZones() || 
        contract->GetDataRequest()->MayRequireNodes() ||
        origOnly)
    {
        keepNodeZone = true;
        if (data.ValidActiveVariable())
        {
            if (data.GetCentering() == AVT_NODECENT)
            {
                rv->GetDataRequest()->TurnNodeNumbersOn();
            }
            else if (data.GetCentering() == AVT_ZONECENT)
            {
                rv->GetDataRequest()->TurnZoneNumbersOn();
            }
        }
        else 
        {
            // canot determine variable centering, so turn on both
            // node numbers and zone numbers.
            rv->GetDataRequest()->TurnNodeNumbersOn();
            rv->GetDataRequest()->TurnZoneNumbersOn();
        }
    }
    else
    {
        keepNodeZone = false;
    }

    return rv;
}
