/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                            avtBestFitLineQuery.C                          //
// ************************************************************************* //

#include <avtBestFitLineQuery.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>

#include <avtCallback.h>
#include <avtParallel.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <snprintf.h>

using     std::string;

#define N_SUM   0
#define X_SUM   1
#define Y_SUM   2
#define XY_SUM  3
#define X2_SUM  4
#define Y2_SUM  5


// ****************************************************************************
//  Method: avtBestFitLineQuery constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Nov 16 14:38:19 PST 2005
//
// ****************************************************************************

avtBestFitLineQuery::avtBestFitLineQuery()
{
    sums[N_SUM] = 0.;
    sums[X_SUM] = 0.;
    sums[Y_SUM] = 0.;
    sums[XY_SUM] = 0.;
    sums[X2_SUM] = 0.;
    sums[Y2_SUM] = 0.;

    strcpy(descriptionBuffer, "Fitting line to points");
}


// ****************************************************************************
//  Method: avtBestFitLineQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Nov 16 14:38:19 PST 2005
//
// ****************************************************************************

avtBestFitLineQuery::~avtBestFitLineQuery()
{
    ;
}


// ****************************************************************************
//  Method: avtBestFitLineQuery::PreExecute
//
//  Purpose:
//      This is called before all of the domains are executed.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Nov 16 14:38:19 PST 2005
//
// ****************************************************************************

void
avtBestFitLineQuery::PreExecute(void)
{
    sums[N_SUM] = 0.;
    sums[X_SUM] = 0.;
    sums[Y_SUM] = 0.;
    sums[XY_SUM] = 0.;
    sums[X2_SUM] = 0.;
    sums[Y2_SUM] = 0.;
}


// ****************************************************************************
//  Method: avtBestFitLineQuery::PostExecute
//
//  Purpose:
//      This is called after all of the domains are executed.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Nov 16 14:38:19 PST 2005
//
//  Modifications:
//
// ****************************************************************************

void
avtBestFitLineQuery::PostExecute(void)
{
    double d[6];
    SumDoubleArrayAcrossAllProcessors(sums, d, 6);

    debug4 << "avtBestFitLineQuery::PostExecute: \n"
           << "\tN_SUM=" << d[N_SUM] << endl
           << "\tX_SUM=" << d[X_SUM] << endl
           << "\tY_SUM=" << d[Y_SUM] << endl
           << "\tXY_SUM=" << d[XY_SUM] << endl
           << "\tX2_SUM=" << d[X2_SUM] << endl
           << "\tY2_SUM=" << d[Y2_SUM]
           << endl;

    double dY = (d[N_SUM] * d[XY_SUM] - d[X_SUM] * d[Y_SUM]);
    double dX = (d[N_SUM] * d[X2_SUM] - d[X_SUM] * d[X_SUM]);
    double m, b, r;
    std::string s;

    if(dX == 0.)
    {
        double x = d[X_SUM] / d[N_SUM];
        m = DBL_MAX;
        r = 1.;

        char buf[1024];
        SNPRINTF(buf, 1024, "The best fit line is: X = %g with a "
                 "correlation coefficient of: %g", x, r);
        s = std::string(buf);
    }
    else
    {
        m =  dY / dX;
        b = (d[Y_SUM] - m * d[X_SUM]) / d[N_SUM];

        double rtop = d[N_SUM] * d[XY_SUM] - d[X_SUM] * d[Y_SUM];
        double r0 = d[N_SUM] * d[X2_SUM] - d[X_SUM] * d[X_SUM];
        double r1 = d[N_SUM] * d[Y2_SUM] - d[Y_SUM] * d[Y_SUM];
        double rbottom = r0 * r1;
        if (rbottom > 0.)
            r = rtop / sqrt(rbottom);

        // Create a return message.
        char buf[1024];
        SNPRINTF(buf, 1024, "The best fit line is: Y = %gX ", m);
        s = std::string(buf);
        if(b < 0.)
        {
            SNPRINTF(buf, 1024, "-%g ", b);
            buf[1] = ' ';
            s += buf;
        }
        else if(b > 0.)
        {
            SNPRINTF(buf, 1024, "+ %g ", b);
            s += buf;
        }

        if (rbottom > 0.)
            SNPRINTF(buf, 1024, "with a correlation coefficient of: %g", r);
        else
            SNPRINTF(buf, 1024, "with an undefined correlation coefficient.");
        s += buf;
    }
  
    //
    //  Parent class uses this message to set the Results message
    //  in the Query Attributes that is sent back to the viewer.
    //  That is all that is required of this query.
    //
    SetResultMessage(s);

    doubleVector retval;
    retval.push_back(m);
    retval.push_back(b);
    retval.push_back(r);
    SetResultValues(retval);
}


// ****************************************************************************
//  Method: avtBestFitLineQuery::Execute
//
//  Purpose:
//      Processes a single domain.
//
//  Programmer: Brad Whitlock  
//  Creation:   Wed Nov 16 14:38:19 PST 2005
//
//  Modifications:
//    Kathleen Bonnell, Mon Jul 31 08:19:38 PDT 2006
//    Support 1D RectilinearGrids (new representation of curves).
//
// ****************************************************************************

void
avtBestFitLineQuery::Execute(vtkDataSet *ds, const int dom)
{
    bool rgrid1D = false;
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        rgrid1D = (((vtkRectilinearGrid*)ds)->GetDimensions()[1] <= 1);
    }
    if (!rgrid1D)
    {
        vtkIdType npts = ds->GetNumberOfPoints();

        sums[N_SUM] += double(npts);
        for(vtkIdType i = 0; i < npts; ++i)
        {
            double fptr[3];
            ds->GetPoint(i, fptr);

            sums[X_SUM] += double(fptr[0]);
            sums[Y_SUM] += double(fptr[1]);
            sums[XY_SUM] += double(fptr[0] * fptr[1]);
            sums[X2_SUM] += double(fptr[0] * fptr[0]);
            sums[Y2_SUM] += double(fptr[1] * fptr[1]);
        }
    }
    else
    {
        vtkDataArray *xc = ((vtkRectilinearGrid*)ds)->GetXCoordinates();
        vtkDataArray *yc = ((vtkRectilinearGrid*)ds)->GetPointData()->GetScalars();
      
        int npts = xc->GetNumberOfTuples();

        sums[N_SUM] += double(npts);
        for(vtkIdType i = 0; i < npts; ++i)
        {
            double x = xc->GetTuple1(i);
            double y = yc->GetTuple1(i);

            sums[X_SUM] += double(x);
            sums[Y_SUM] += double(y);
            sums[XY_SUM] += double(x*y); 
            sums[X2_SUM] += double(x*x); 
            sums[Y2_SUM] += double(y*y); 
        }
    }
}


