// ************************************************************************* //
//                            avtBestFitLineQuery.C                          //
// ************************************************************************* //

#include <avtBestFitLineQuery.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
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
//
// ****************************************************************************

void
avtBestFitLineQuery::Execute(vtkDataSet *ds, const int dom)
{
    vtkIdType npts = ds->GetNumberOfPoints();

    sums[N_SUM] += double(npts);
    for(vtkIdType i = 0; i < npts; ++i)
    {
        float fptr[3];
        ds->GetPoint(i, fptr);

        sums[X_SUM] += double(fptr[0]);
        sums[Y_SUM] += double(fptr[1]);
        sums[XY_SUM] += double(fptr[0] * fptr[1]);
        sums[X2_SUM] += double(fptr[0] * fptr[0]);
        sums[Y2_SUM] += double(fptr[1] * fptr[1]);
    }
}


