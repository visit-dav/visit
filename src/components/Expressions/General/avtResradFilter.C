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
//                               avtResradFilter.C                           //
// ************************************************************************* //

#include <avtResradFilter.h>

#include <vtkDataArray.h>
#include <vtkRectilinearGrid.h>

#include <avtCallback.h>

#include <ExpressionException.h>


static bool varres(float *var, float *newvar, int nxvar, int nyvar, 
                   int reflflag, float dx, float dy, float resrad);


// ****************************************************************************
//  Method: avtResradFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

avtResradFilter::avtResradFilter()
{
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtResradFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

avtResradFilter::~avtResradFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtResradFilter::PreExecute
//
//  Purpose:
//      Initialize the haveIssuedWarning flag.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

void
avtResradFilter::PreExecute(void)
{
    avtBinaryMathFilter::PreExecute();
    haveIssuedWarning = false;
}


// ****************************************************************************
//  Method: avtResradFilter::DoOperation
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
// ****************************************************************************

void
avtResradFilter::DoOperation(vtkDataArray *in1, vtkDataArray *in2,
                                vtkDataArray *out, int ncomponents,int ntuples)
{
    int  i;

    float *var = (float *) in1->GetVoidPointer(0);
    float radius = in2->GetTuple1(0);  // Assuming constant.
    float *newvar = (float *) out->GetVoidPointer(0);

    //
    // There are so many ways to go wrong, initialize the array assuming we
    // will return with an error condition.
    //
    int nvals = out->GetNumberOfTuples();
    for (i = 0 ; i < nvals ; i++)
         newvar[i] = var[i];

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

    float *X = (float *) rgrid->GetXCoordinates()->GetVoidPointer(0);
    float *Y = (float *) rgrid->GetYCoordinates()->GetVoidPointer(0);

    bool hasEqualSpacing = true;
    float dx = X[1] - X[0];
    float fudge = dx/1000.;
    for (i = 1 ; i < dims[0]-1 ; i++)
        if (fabs((X[i+1] - X[i]) - dx) > fudge)
            hasEqualSpacing = false;
    float dy = Y[1] - Y[0];
    fudge = dy/1000.;
    for (i = 1 ; i < dims[1]-1 ; i++)
        if (fabs((Y[i+1] - Y[i]) - dy) > fudge)
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
    if (!varres(var, newvar, nX, nY, 1, dx, dy, radius))
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

#define NPG   200
#define NGPS  2000
#define TPI   6.283185308

#define NRES 8
#define RES(k,j)    res[((j)+8)*(NRES+NRES+1)+((k)+8)]
#define VAR(i,j)    var[((j)-1)*nxvar+((i)-1)]
#define NEWVAR(i,j) newvar[((j)-1)*nxvar+((i)-1)]

#ifndef MIN
#define MIN(a,b) ((a < b) ? a : b)
#endif
#ifndef MAX
#define MAX(a,b) ((a > b) ? a : b)
#endif

static float     res[(NRES+NRES+1)*(NRES+NRES+1)];
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

bool
varres (float *var, float *newvar, int nxvar, int nyvar, int reflflag,
        float dx, float dy, float resrad)
{
     int       i, j, jj, jm, j2, k, kk, k1, k2;
     int       jext, kext;
     int       nresp1;
     float     fnresp1;
     float     rr, phi, x, y, avg;
     int       ngroup;
     float     randnum;
     float     pnormi;
     float     rr2;

     pnormi = 1. / (float) (NGPS * NPG);
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
          j2 = MIN (nyvar, nyvar - jj);

          for (kk = -kext; kk <= kext; kk++) {
               k1 = MAX (1, 1 - kk);
               k2 = MIN (nxvar, nxvar - kk);

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


