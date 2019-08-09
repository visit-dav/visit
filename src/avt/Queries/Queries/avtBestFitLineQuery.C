// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
// ****************************************************************************

void
avtBestFitLineQuery::PreExecute(void)
{
    avtSummationQuery::PreExecute();

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
//    Cyrus Harrison, Tue Sep 18 13:45:35 PDT 2007
//    Added support for user settable floating point format string
//
//    Kathleen Biagas, Tue Feb 25 16:07:00 PST 2014
//    Add Xml results.
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
    string floatFormat = queryAtts.GetFloatFormat();
    string format;
    char buf[1024];
    MapNode result_node;

    if(dX == 0.)
    {
        double x = d[X_SUM] / d[N_SUM];
        m = DBL_MAX;
        r = 1.;
       
        format = "The best fit line is: X = " + floatFormat 
                  + " with a correlation coefficient of: "+ floatFormat;
        snprintf(buf, 1024, format.c_str(), x, r);
        s = std::string(buf);
        result_node["X"] = x;
        result_node["correlation_coefficient"] = r;
    }
    else
    {
        m =  dY / dX;
        b = (d[Y_SUM] - m * d[X_SUM]) / d[N_SUM];

        MapNode slopeIntercept;
        slopeIntercept["m"] = m;
        slopeIntercept["b"] = b;
        result_node["Y"] = slopeIntercept;

        double rtop = d[N_SUM] * d[XY_SUM] - d[X_SUM] * d[Y_SUM];
        double r0 = d[N_SUM] * d[X2_SUM] - d[X_SUM] * d[X_SUM];
        double r1 = d[N_SUM] * d[Y2_SUM] - d[Y_SUM] * d[Y_SUM];
        double rbottom = r0 * r1;
        if (rbottom > 0.)
            r = rtop / sqrt(rbottom);

        // Create a return message.
        format = "The best fit line is: Y = " + floatFormat  + "X ";
        snprintf(buf, 1024,format.c_str(), m);
        s = std::string(buf);
        if(b < 0.)
        {
            format = "-" + floatFormat + " "; 
            snprintf(buf, 1024, format.c_str(), b);
            buf[1] = ' ';
            s += buf;
        }
        else if(b > 0.)
        {
            format = "+ " + floatFormat + " "; 
            snprintf(buf, 1024, format.c_str(), b);
            s += buf;
        }

        if (rbottom > 0.)
        {
            format = "with a correlation coefficient of: " + floatFormat;
            snprintf(buf, 1024, format.c_str(), r);
            s += buf;
            result_node["correlation_coefficient"] = r;
        }
        else
        {
            s += "with an undefined correlation coefficient.";
            result_node["correlation_coefficient"] = string("undefined");
        }
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
    SetXmlResult(result_node.ToXML());
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


