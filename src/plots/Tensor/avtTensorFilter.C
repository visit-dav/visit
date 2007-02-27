/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                             avtTensorFilter.C                             //
// ************************************************************************* //

#include <avtTensorFilter.h>

#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkTensorReduceFilter.h>
#include <vtkVertexFilter.h>

// ****************************************************************************
//  Method: avtTensorFilter constructor
//
//  Arguments:
//      us      A boolean indicating if you should use the stride (true) or
//              stick with a fixed number of tensors (false).
//      red     The reduction factor, the stride of number of tensors depending
//              on us.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//  Modifications:
//    Kathleen Bonnell, Tue Aug 30 15:11:01 PDT 2005
//    Added keepNodeZone.
//
// ****************************************************************************

avtTensorFilter::avtTensorFilter(bool us, int red)
{
    reduce = vtkTensorReduceFilter::New();
    vertex = vtkVertexFilter::New();

    if (us)
    {
        SetStride(red);
    }
    else
    {
        SetNTensors(red);
    }

    if (useStride)
    {
        reduce->SetStride(stride);
    }
    else
    {
        reduce->SetNumberOfElements(nTensors);
    }
    keepNodeZone = false;
}


// ****************************************************************************
//  Method: avtTensorFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Aug 30 17:30:48 PDT 2001
//    Added vertex filter.
//
// ****************************************************************************

avtTensorFilter::~avtTensorFilter()
{
    if (reduce != NULL)
    {
        reduce->Delete();
    }
    if (vertex != NULL)
    {
        vertex->Delete();
    }
}


// ****************************************************************************
//  Method: avtTensorFilter::SetStride
//
//  Purpose:
//      Sets the stride of reduction for the vector.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
// ****************************************************************************

void
avtTensorFilter::SetStride(int s)
{
    useStride = true;
    stride    = s;
    nTensors  = -1;
    reduce->SetStride(stride);
}


// ****************************************************************************
//  Method: avtTensorFilter::SetNTensors
//
//  Purpose:
//      Sets the number of vectors the filter should try to output.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
// ****************************************************************************

void
avtTensorFilter::SetNTensors(int n)
{
    useStride = false;
    stride    = -1;
    nTensors  = n;
    reduce->SetNumberOfElements(nTensors);
}


// ****************************************************************************
//  Method: avtTensorFilter::Equivalent
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
//  Creation:    September 23, 2003
//
// ****************************************************************************

bool
avtTensorFilter::Equivalent(bool us, int red)
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
        if (red != nTensors)
        {
            return false;
        }
    }

    return true;
}


// ****************************************************************************
//  Method: avtTensorFilter::ExecuteData
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
//  Creation:     September 23, 2003
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
// ****************************************************************************

vtkDataSet *
avtTensorFilter::ExecuteData(vtkDataSet *inDS, int, std::string)
{
    vtkPolyData *outPD = vtkPolyData::New();

    if (inDS->GetPointData()->GetTensors() != NULL)
    {
        vertex->VertexAtPointsOn();
    }
    else
    {
        vertex->VertexAtPointsOff();
    }

    vertex->SetInput(inDS);
    reduce->SetInput(vertex->GetOutput());
    reduce->SetOutput(outPD);
    outPD->Delete();
    outPD->Update();

    return outPD;
}


// ****************************************************************************
//  Method: avtTensorFilter::RefashionDataObjectInfo
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
//    Kathleen Bonnell, Tue Aug 30 15:11:01 PDT 2005
//    Added keepNodeZone.
//
// ****************************************************************************

void
avtTensorFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(0);
    GetOutput()->GetInfo().GetValidity().SetNormalsAreInappropriate(true);
    GetOutput()->GetInfo().GetAttributes().SetKeepNodeZoneArrays(keepNodeZone);
}


// ****************************************************************************
//  Method: avtTensorFilter::ReleaseData
//
//  Purpose:
//      Releases all problem size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Mar  4 08:12:25 PST 2005
//    Do not set outputs of filters to NULL, since this will prevent them
//    from re-executing correctly in DLB-mode.
//
//    Kathleen Bonnell, Wed May 18 15:07:05 PDT 2005 
//    Fix memory leak. 
//
// ****************************************************************************

void
avtTensorFilter::ReleaseData(void)
{
    avtStreamer::ReleaseData();

    reduce->SetInput(NULL);
    vtkPolyData *p = vtkPolyData::New();
    reduce->SetOutput(p);
    p->Delete();
    vertex->SetInput(NULL);
    p = vtkPolyData::New();
    vertex->SetOutput(p);
    p->Delete();
}


// ****************************************************************************
//  Method: avtTensorFilter::PerformRestriction
//
//  Purpose:  
//    Request original nodes/zones when appropriate. 
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   August 30, 2005 
//
//  Modifications:
//
// ****************************************************************************

avtPipelineSpecification_p
avtTensorFilter::PerformRestriction(avtPipelineSpecification_p pspec)
{
    avtPipelineSpecification_p rv = pspec;

    if (pspec->GetDataSpecification()->MayRequireZones() || 
        pspec->GetDataSpecification()->MayRequireNodes())
    {
        avtDataAttributes &data = GetInput()->GetInfo().GetAttributes();
        keepNodeZone = true;
        if (data.ValidActiveVariable())
        {
            if (data.GetCentering() == AVT_NODECENT)
            {
                rv->GetDataSpecification()->TurnNodeNumbersOn();
            }
            else if (data.GetCentering() == AVT_ZONECENT)
            {
                rv->GetDataSpecification()->TurnZoneNumbersOn();
            }
        }
        else 
        {
            // canot determine variable centering, so turn on both
            // node numbers and zone numbers.
            rv->GetDataSpecification()->TurnNodeNumbersOn();
            rv->GetDataSpecification()->TurnZoneNumbersOn();
        }
    }
    else
    {
        keepNodeZone = false;
    }

    return rv;
}



