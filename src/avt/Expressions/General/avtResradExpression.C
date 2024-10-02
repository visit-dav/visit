// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtResradExpression.C                           //
// ************************************************************************* //

#include <avtResradExpression.h>

#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>

#include <avtCallback.h>

#include <ExpressionException.h>

template <typename T>
static bool varres(T *var, T *newvar, int nxvar, int nyvar, 
                   int reflflag, double dx, double dy, double resrad);


// ****************************************************************************
//  Method: avtResradExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

avtResradExpression::avtResradExpression()
{
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtResradExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

avtResradExpression::~avtResradExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtResradExpression::PreExecute
//
//  Purpose:
//      Initialize the haveIssuedWarning flag.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

void
avtResradExpression::PreExecute(void)
{
    avtBinaryMathExpression::PreExecute();
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtResradExpression::DoOperation
//
//  Purpose:
//      Calculates the resrad.  This adjusts the resolution using a monte
//      carlo resampling.
//
//  Arguments:
//      in1           The first input data array.
//      in2           The second input data array.
//      out           The output data array.
//      ncomponents   The number of components ('1' for scalar, '2' or '3' for
//                    vectors, etc.)
//      ntuples       The number of tuples (ie 'npoints' or 'ncells')
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
//  Modifications:
//    Brad Whitlock, Thu Feb 24 16:17:28 PST 2005
//    Fixed i redefinition for win32.
//
//    Eric Brugger, Tue May 23 12:22:57 PDT 2017
//    I seeded the random number generator to get reproducible results. I
//    just used zero instead of the domain id, since this expression only
//    works on single block rectilinear meshes.
//
// ****************************************************************************

void
avtResradExpression::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                vtkDataArray *out, int ncomponents,int ntuples)
{
    //
    // There are so many ways to go wrong, initialize the array assuming we
    // will return with an error condition.
    //
    vtkIdType nvals = out->GetNumberOfTuples();
    for (vtkIdType i = 0 ; i < nvals ; i++)
         out->SetTuple1(i, in1->GetTuple1(i));

    if (cur_mesh->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("The resrad expression only operates on"
                                      " 2D rectilinear meshes.");
            haveIssuedWarning = true;
        }
        return;
    }

    int dims[3];
    vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) cur_mesh;
    rgrid->GetDimensions(dims);
    if (dims[2] != 1)
    {
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("The resrad expression only operates on"
                                      " 2D rectilinear meshes.");
            haveIssuedWarning = true;
        }
        return;
    }

    vtkDataArray *X = rgrid->GetXCoordinates();
    vtkDataArray *Y = rgrid->GetYCoordinates();

    bool hasEqualSpacing = true;
    double dx = X->GetTuple1(1) - X->GetTuple1(0);
    double fudge = dx/1000.;
    for (int i = 1 ; i < dims[0]-1 ; i++)
        if (fabs((X->GetTuple1(i+1) - X->GetTuple1(i)) - dx) > fudge)
            hasEqualSpacing = false;
    double dy = Y->GetTuple1(1) - Y->GetTuple1(0);
    fudge = dy/1000.;
    for (int i = 1 ; i < dims[1]-1 ; i++)
        if (fabs((Y->GetTuple1(i+1) - Y->GetTuple1(i)) - dy) > fudge)
            hasEqualSpacing = false;

    if (!hasEqualSpacing)
    {
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("The spacing of the rectilinear grid "
                                      "must be regular (equidistant).");
            haveIssuedWarning = true;
        }
        return;
    }

    int nX = dims[0];
    int nY = dims[1];
    if (centering != AVT_NODECENT)
    {
        nX--;
        nY--;
    }

    //
    // Seed the random number generator for reproducibility.
    //
    srand(0);

    double radius = in2->GetTuple1(0);  // Assuming constant.
    bool good = false;
    if (in1->GetDataType() == VTK_FLOAT)
    {
        good = varres<float>(vtkFloatArray::SafeDownCast(in1)->GetPointer(0), 
                             vtkFloatArray::SafeDownCast(out)->GetPointer(0),
                             nX, nY, 1, dx, dy, radius);
    }
    else if (in1->GetDataType() == VTK_DOUBLE)
    {
        good = varres<double>(vtkDoubleArray::SafeDownCast(in1)->GetPointer(0), 
                              vtkDoubleArray::SafeDownCast(out)->GetPointer(0),
                              nX, nY, 1, dx, dy, radius);
    }

    if (!good)
    {
        if (!haveIssuedWarning)
        {
            avtCallback::IssueWarning("The resrad expression failed.  This is"
                   "probably because the radius you specified is too large.");
            haveIssuedWarning = true;
        }
        return;
    }
}


// The below all comes from the MeshTV file qrvarres where resrad was
// originally implemented.

#include <stdlib.h>
#include <math.h>
#include <algorithm>

#define NPG   200
#define NGPS  2000
#define TPI   6.283185308

#define NRES 8
#define RES(k,j)    res[((j)+8)*(NRES+NRES+1)+((k)+8)]
#define VAR(i,j)    var[((j)-1)*nxvar+((i)-1)]
#define NEWVAR(i,j) newvar[((j)-1)*nxvar+((i)-1)]

static double    res[(NRES+NRES+1)*(NRES+NRES+1)];
static int       kx [NPG], jy [NPG];


/*-------------------------------------------------------------------------
 * Function:    randf
 *
 * Purpose:     Generates random numbers between RMIN (inclusive) and
 *              RMAX (exclusive).  RMIN should be smaller than RMAX.
 *
 * Return:      A pseudo-random number
 *
 * Programmer:  Robb Matzke
 *              robb@callisto.nuance.mdn.com
 *              Jul  9, 1996
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static double
randf(double rmin, double rmax)
{
    unsigned long   acc;
    static double   divisor = 0;

    if (divisor < 1)
        divisor = pow(2.0, 30);

    rmax -= rmin;
    acc = ((rand() & 0x7fff) << 15) | (rand() & 0x7fff);

    return (rmax * (acc / divisor) + rmin);
}


/***********************************************************************
 *  PURPOSE    -- Adjust resolution of regular grid data using
 *                Monte Carlo algo.
 *
 *  PROGRAMMER -- Algorithm: Gary Carlson
 *                Minor coding/comment changes:  Jeffery W. Long, NSSD-B
 *
 *  PARAMETERS
 *
 *     var       =|    original (raw) var data
 *     newvar     |=   array to hold new (revised) var data
 *     nxvar     =|    number of var bins in the z-direction
 *     nyvar     =|    number of var bins in the r-direction
 *     reflflag  =|    special reflection flag (1=on)
 *     dx        =|    delta (width) for z bins
 *     dy        =|    delta (width) for r bins
 *     resrad    =|    resolution of new varhole (in cm.)
 *     ret value  |=   error indicator
 *
 *  MODIFICATIONS
 *     Al Leibee, Mon Aug  2 10:32:58 PDT 1993
 *     Fixed 'randfact'.
 *
 *     Al Leibee, Thu Jun 23 13:20:43 PDT 1994
 *     Fixed 'randfact'.
 *
 *     Eric Brugger, Wed Mar  1 10:44:17 PST 1995
 *     I further fixed 'randfact' for the sun.
 *
 *     Sean Ahern, Wed Apr 24 14:17:46 PDT 1996
 *     Added more return values for better error reporting.
 *
 *     Sean Ahern, Wed Jul 10 15:51:15 PDT 1996
 *     To finalize things... :-)  I removed randfact altogether.  It's not
 *     needed since we aren't using rand any more.
 *
 **********************************************************************/
template <typename T>
bool
varres (T *var, T *newvar, int nxvar, int nyvar, int reflflag,
        double dx, double dy, double resrad)
{
     int       i, j, jj, jm, j2, k, kk, k1, k2;
     int       jext, kext;
     int       nresp1;
     double    fnresp1;
     double    rr, phi, x, y, avg;
     int       ngroup;
     double    randnum;
     double    pnormi;
     double    rr2;

     pnormi = 1. / (double) (NGPS * NPG);
     rr2    = resrad * resrad;

     for (i = 0; i < nxvar * nyvar; i++)
          newvar [i] = 0.0;

     for (i = 0; i < (2 * NRES + 1) * (2 * NRES + 1); i++)
          res [i] = 0.0;

     /*
      * Check for legal resolution ratio
      */

     jext = (int) (resrad / dy + 1.);
     kext = (int) (resrad / dx + 1.);

     if (jext<1)
         return false;
    if (kext<1)
        return false;
    if (jext>NRES)
        return false;
    if (kext>NRES)
        return false;

     /*
      * use monte carlo method to find resolution function
      */

     nresp1  = NRES + 1;
     fnresp1 = nresp1;

     for (ngroup = 1; ngroup < NGPS; ngroup++)
     {
        for (i = 0; i < NPG; i++)
        {
            randnum = randf(0.0,1.0);
            rr      = sqrt (rr2 * randnum);
            randnum = randf(0.0,1.0);
            phi     = randnum * TPI;
            randnum = randf(0.0,1.0);
            x       = fnresp1 +  randnum + rr * cos (phi) / dx;
            randnum = randf(0.0,1.0);
            y       = fnresp1 +  randnum + rr * sin (phi) / dy;
            kx [i]  = ((int) x) - nresp1;
            jy [i]  = ((int) y) - nresp1;
        }

        for (i = 0; i < NPG; i++)
            RES (kx[i],jy[i]) = RES (kx[i],jy[i]) + pnormi;
     }

     for (j = 1; j <= NRES; j++) {
          for (k = -NRES; k <= NRES; k++) {
               avg        = .5 * (RES (j,k) + RES (-j,k));
               RES (j,k)  = avg;
               RES (-j,k) = avg;
          }
     }

     for (k = 1; k <= NRES; k++) {
          for (j = -NRES; j <= NRES; j++) {
               avg         = .5 * (RES (j,k) + RES (j,-k));
               RES (j,k)   = avg;
               RES (j,-k)  = avg;
          }
     }

     /*
      * Calculate resolution effect on var
      */
     for (jj = -jext; jj <= jext; jj++) {
          j2 = std::min(nyvar, nyvar - jj);

          for (kk = -kext; kk <= kext; kk++) {
               k1 = std::max(1, 1 - kk);
               k2 = std::min(nxvar, nxvar - kk);

               for (j = 1; j <= j2; j++) {
                    jm = j + jj;
                    if (jm < 1)
                         jm = 1 - jm;

                    for (k = k1; k <= k2; k++)
                         NEWVAR (k,j) += VAR (k+kk, jm) * RES (kk,jj);

                    if (reflflag == 1. && kk < 0) {
                         for (k = 1; k <= -kk; k++)
                              NEWVAR (k,j) += VAR (-kk + 1 - k, jm) *
                                    RES (kk, jj);
                    }
               }
          }
     }

     return true;
}


