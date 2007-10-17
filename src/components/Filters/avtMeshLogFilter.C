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
//                         avtMeshLogFilter.C                                //
// ************************************************************************* // 

#include <math.h>

#include <avtMeshLogFilter.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>
#include <vtkPointSet.h>
#include <vtkRectilinearGrid.h>
#include <avtExtents.h>


// ****************************************************************************
//  Method: avtMeshLogFilter::Constructor
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 6, 2007 
//
// ****************************************************************************

avtMeshLogFilter::avtMeshLogFilter()
{
    xScaleMode = LINEAR;
    yScaleMode = LINEAR;
    useInvLogX = false;
    useInvLogY = false;
}


// ****************************************************************************
//  Method: avtMeshLogFilter::Execute
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 6, 2007 
//
// ****************************************************************************

vtkDataSet *
avtMeshLogFilter::ExecuteData(vtkDataSet *ds, int, std::string)
{
    if (xScaleMode == LINEAR && yScaleMode == LINEAR)
        return ds;

    vtkDataSet *rv = ds->NewInstance();
    rv->ShallowCopy(ds);
        
    if (rv->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        if (xScaleMode == LOG)
        {
            vtkDataArray *xc = ((vtkRectilinearGrid*)rv)->GetXCoordinates();
            float *x = (float*)((vtkFloatArray*)xc)->GetVoidPointer(0);
            for (int i = 0; i < xc->GetNumberOfTuples(); i++)
                ScaleVal(x[i], useInvLogX);
            ((vtkRectilinearGrid*)rv)->SetXCoordinates(xc);
        }
        if (yScaleMode == LOG)
        {
            vtkDataArray *yc = ((vtkRectilinearGrid*)rv)->GetYCoordinates();
            float *y = (float*)((vtkFloatArray*)yc)->GetVoidPointer(0);
            for (int i = 0; i < yc->GetNumberOfTuples(); i++)
                ScaleVal(y[i], useInvLogY);
            ((vtkRectilinearGrid*)rv)->SetYCoordinates(yc);
        }
    }
    else 
    {
        vtkDataArray *points = ((vtkPointSet*)ds)->GetPoints()->GetData();
        float *pts = (float*)((vtkFloatArray*)points)->GetVoidPointer(0);
        for (int i = 0; i < points->GetNumberOfTuples()*3; i+=3)
        {
            if (xScaleMode == LOG)
            {
                ScaleVal(pts[i], useInvLogX);
            }
            if (yScaleMode == LOG)
            {
                ScaleVal(pts[i+1], useInvLogY);
            }
        } 
        ((vtkPointSet*)rv)->GetPoints()->SetData(points);
    }
    ManageMemory(rv);
    return rv;
}


// ****************************************************************************
//  Method: avtMeshLogFilter::PostExecute
//
//  Purpose:  Ensures correct extents get passed along
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 6, 2007 
//
// ****************************************************************************

void
avtMeshLogFilter::PostExecute()
{
    avtStreamer::PostExecute();

    avtDataAttributes& inAtts  = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes& outAtts = GetOutput()->GetInfo().GetAttributes();

    // over-write spatial extents
    outAtts.GetTrueSpatialExtents()->Clear();
    outAtts.GetCumulativeTrueSpatialExtents()->Clear();

    // get the outputs's spatial extents
    double se[6];
    if (inAtts.GetTrueSpatialExtents()->HasExtents())
    {
        inAtts.GetTrueSpatialExtents()->CopyTo(se);
        if (xScaleMode == LOG)
        {
            ScaleVal(se[0], useInvLogX);
            ScaleVal(se[1], useInvLogX);
        }
        if (yScaleMode == LOG)
        {
            ScaleVal(se[2], useInvLogY);
            ScaleVal(se[3], useInvLogY);
        }
        outAtts.GetTrueSpatialExtents()->Set(se);
    }
    else if (inAtts.GetCumulativeTrueSpatialExtents()->HasExtents())
    {
        inAtts.GetCumulativeTrueSpatialExtents()->CopyTo(se);
        if (xScaleMode == LOG)
        {
            ScaleVal(se[0], useInvLogX);
            ScaleVal(se[1], useInvLogX);
        }
        if (yScaleMode == LOG)
        {
            ScaleVal(se[2], useInvLogY);
            ScaleVal(se[3], useInvLogY);
        }
        outAtts.GetCumulativeTrueSpatialExtents()->Set(se);
    }
}


// ****************************************************************************
//  Method: avtMeshLogFilter::RefashionDataObjectInfo
//
//  Purpose:
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 6, 2007 
//
// ****************************************************************************

void
avtMeshLogFilter::RefashionDataObjectInfo(void)
{
    avtDataValidity &va = GetOutput()->GetInfo().GetValidity();
    va.InvalidateSpatialMetaData();
    va.SetPointsWereTransformed(true);
}


#define SMALL 1e-100

void
avtMeshLogFilter::ScaleVal(float &v, bool invLog)
{
    if (invLog)
        return ScaleVal_invlog(v);
    else 
        return ScaleVal_log(v);
}

void
avtMeshLogFilter::ScaleVal(double &v, bool invLog)
{
    if (invLog)
        return ScaleVal_invlog(v);
    else 
        return ScaleVal_log(v);
}


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Tue Oct 16 16:16:34 PDT 2007
//    Remove fabs call.
//
// ****************************************************************************

void
avtMeshLogFilter::ScaleVal_log(float &v)
{
    v = log10(fabs(v) + SMALL);
}

void
avtMeshLogFilter::ScaleVal_log(double &v)
{
    v = log10(fabs(v) + SMALL);
}


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Tue Oct 16 16:16:34 PDT 2007
//    Remove fabs call.
//
//    Hank Childs, Wed Oct 17 16:04:19 PDT 2007
//    Make sure both arguments to pow are doubles.  Otherwise, this causes 
//    an ambiguity that xlc can't handle.
//
// ****************************************************************************

void
avtMeshLogFilter::ScaleVal_invlog(float &v)
{
    v = pow(10., (double) v);
}

void
avtMeshLogFilter::ScaleVal_invlog(double &v)
{
    v = pow(10., v);
}

