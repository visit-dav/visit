/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
#include <VolumeFunctions.h>
#include <math.h>
#include <float.h>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>

#include <vtkDelaunay2D.h>
#include <vtkDelaunay3D.h>
#include <vtkUnstructuredGrid.h>
#include <vtkSmartPointer.h>
#include <vtkIdList.h>
#include <vtkCell.h>
#include <vtkPolyData.h>
#include <DebugStream.h>
#include <StackTimer.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>

#include <vtkSkew.h>
#include <avtAccessor.h>

#include <VolumeAttributes.h>
#include <InvalidLimitsException.h>
#include <ImproperUseException.h>
#include <string>

const double PI = atan(1.0)*4.0;

#define NO_DATA_VALUE -1e+37

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

// ****************************************************************************
//  Method: VolumeGetRange
//
//  Purpose:
//      Determines the range for a scalar variable.
//
//  Arguments:
//      s       The scalar variable.  This variable may have some dummy values
//              (like NO_DATA_VALUE, etc).
//      min     Will contain the minimum after execution.
//      max     Will contain the maximum after execution.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Brad Whitlock, Wed Dec 17 11:22:03 PST 2008
//    I added a faster path for float data.
//
// ****************************************************************************

template <class Accessor>
void
VolumeGetRange_Impl(vtkDataArray *s, float &min, float &max)
{
    Accessor a(s);
    while(a.Iterating())
    {
        float v = a.GetTuple1();
        a++;
        if (v < NO_DATA_VALUE)
            continue;
        if (v < min)
            min = v;
        if (v > max)
            max = v;
    }
}

void
VolumeGetRange(vtkDataArray *s, float &min, float &max)
{
    StackTimer t("VolumeGetRange");

    min = +FLT_MAX;
    max = -FLT_MAX;
    if(s->GetDataType() == VTK_FLOAT)
         VolumeGetRange_Impl<avtDirectAccessor<float> >(s, min, max);
    else
         VolumeGetRange_Impl<avtTupleAccessor>(s, min, max);
}

// ****************************************************************************
// Method: VolumeLogTransform
//
// Purpose: 
//   Computes log on one data array, storing it into another data array.
//
// Arguments:
//   atts   : The Volume plot attributes.
//   linear : The linear values.
//   skew   : The computed log values.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 19 14:02:29 PST 2008
//
// Modifications:
//   
// ****************************************************************************

#if 0
template <class Accessor>
static void
VolumeLogTransform_Impl(vtkDataArray *linear, vtkDataArray *log, 
    float min_range)
{
    Accessor src(linear), dest(log);
    while(src.Iterating())
    {
        float f = src.GetTuple1();
        if (f > 0)
            f = log10(f);
        else if (f > NO_DATA_VALUE)
            f = log10(min_range);

        dest.SetTuple1(f);
        dest++;
        src++;
    }
}

void
VolumeLogTransform(const VolumeAttributes &atts, 
    vtkDataArray *linear, vtkDataArray *log)
{
    double *r = linear->GetRange();
    float range[2];
    range[0] = r[0];
    range[1] = r[1];

    if (atts.GetUseColorVarMin())
    {
        range[0] = atts.GetColorVarMin();
    }
    if (atts.GetUseColorVarMax())
    {
        range[1] = atts.GetColorVarMax();
    }
    if (range[0] <= 0. || range[1] <= 0.)
    {
        EXCEPTION1(InvalidLimitsException, true);
    }
    if(linear->GetDataType() == VTK_FLOAT &&
       log->GetDataType() == VTK_FLOAT)
    {
        VolumeLogTransform_Impl<avtDirectAccessor<float> >(linear,log,range[0]);
    }
    else
        VolumeLogTransform_Impl<avtTupleAccessor>(linear,log,range[0]);
}
#else
void
VolumeLogTransform(const VolumeAttributes &atts, 
    vtkDataArray *linear, vtkDataArray *log)
{
    StackTimer t("VolumeLogTransform");
    double *r = linear->GetRange();
    float range[2];
    range[0] = r[0];
    range[1] = r[1];

    if (atts.GetUseColorVarMin())
    {
        range[0] = atts.GetColorVarMin();
    }
    if (atts.GetUseColorVarMax())
    {
        range[1] = atts.GetColorVarMax();
    }
    if (range[0] <= 0. || range[1] <= 0.)
    {
        EXCEPTION1(InvalidLimitsException, true);
    }
    if(linear->GetDataType() == VTK_FLOAT &&
       log->GetDataType() == VTK_FLOAT)
    {
        const float *src = (const float *)linear->GetVoidPointer(0);
        const float *end = src + linear->GetNumberOfTuples();
        float *dest = (float *)log->GetVoidPointer(0);
        while(src < end)
        {
            float f = *src++;
            if (f > 0)
                f = log10(f);
            else if (f > NO_DATA_VALUE)
                f = log10(range[0]);
            *dest++ = f;
        }
    }
    else
    {
        for (int i = 0 ; i < linear->GetNumberOfTuples() ; i++)
        {
            double f = linear->GetTuple1(i);
            if (f > 0.)
                f = log10(f);
            else if (f > NO_DATA_VALUE)
                f = log10(range[0]);
            log->SetTuple1(i, f);
        }
    }
}
#endif

// ****************************************************************************
// Method: VolumeSkewTransform
//
// Purpose: 
//   Computes skew on one data array, storing it into another data array.
//
// Arguments:
//   atts   : The Volume plot attributes.
//   linear : The linear values.
//   skew   : The computed skew values.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 19 14:02:29 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
VolumeSkewTransform(const VolumeAttributes &atts, 
    vtkDataArray *linear, vtkDataArray *skew)
{
    StackTimer t("VolumeSkewTransform");
    double *r = linear->GetRange();
    float range[2];
    range[0] = (float)r[0];
    range[1] = (float)r[1];

    if (atts.GetUseColorVarMin())
    {
        range[0] = atts.GetColorVarMin();
    }
    if (atts.GetUseColorVarMax())
    {
        range[1] = atts.GetColorVarMax();
    }
    float skewFactor = atts.GetSkewFactor();
    if(linear->GetDataType() == VTK_FLOAT &&
       skew->GetDataType() == VTK_FLOAT)
    {
        const float *src = (const float *)linear->GetVoidPointer(0);
        const float *end = src + linear->GetNumberOfTuples();
        float *dest = (float *)skew->GetVoidPointer(0);
        while(src < end)
            *dest++ = vtkSkewValue(*src++, range[0], range[1], skewFactor);
    }
    else
    {
        for (int i = 0 ; i < linear->GetNumberOfTuples() ; i++)
        {
            float f = linear->GetTuple1(i);
            skew->SetTuple1(i, (float)vtkSkewValue(f, range[0], range[1], skewFactor));
        }
    }
}

// ****************************************************************************
// Method: VolumeGetScalar
//
// Purpose: 
//   Returns the data array that we're using for plotting.
//
// Arguments:
//
// Returns:    
//
// Note:       I separated this out from VolumeGetScalars.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 19 11:47:04 PST 2008
//
// Modifications:
//   Allen Harvey, Wed Jun  6 14:02:58 PDT 2012
//   Change implementation so it looks up the proper array based on the name.
//
//   Brad Whitlock, Mon Aug 20 16:26:27 PDT 2012
//   If no variable name was passed, return the active scalar.
//
// ****************************************************************************

vtkDataArray *
VolumeGetScalar(vtkDataSet *ds, const char *name)
{
    vtkPointData *pd = ds->GetPointData();
    vtkDataArray *data = NULL;

    if(name == NULL)
        data = pd->GetScalars();
    else
    {
        //
        // The data is not set up as the active scalars.  Try to guess what
        // it should be.
        //
        for (int i = 0 ; i < pd->GetNumberOfArrays() ; i++)
        {
            vtkDataArray *arr = pd->GetArray(i);
            if (strcmp(arr->GetName(), name) == 0)
            {
                data = arr;
                break;
            }
        }
    }

    return data;
}

// ****************************************************************************
//  Method: VolumeGetScalars
//
//  Purpose:
//      Gets the scalars from a dataset.
//
//  Arguments:
//      ds      A vtk dataset to get the scalars from.
//      data    The normal data scalar.
//      opac    The scalar with the opacity variable.
//
//  Notes:      data and opac must be freed (->Delete) by the calling function.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Dec 14 11:04:52 PST 2001
//    Determine error condition as early as possible.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Mon Dec 23 08:36:18 PST 2002
//    Do a better job of locating the variable.
//
//    Kathleen Bonnell, Fri Mar  4 13:55:09 PST 2005 
//    Account for different scaling methods. 
//
//    Hank Childs, Tue Feb  6 15:37:12 PST 2007
//    Accurately account for log plotting.  Includes dismissing the -1e+38
//    when looking for negative values.
//
//    Brad Whitlock, Fri Dec 19 16:31:48 PST 2008
//    I isolated this function from the renderer and heavily modified it. The
//    data scaling is now done in the engine so we can histogram it and not
//    have to recalculate it for each render.
//
//    Brad Whitlock, Mon Aug 20 16:27:28 PDT 2012
//    Pass NULL into VolumeGetScalar when we obtain the color variable so we
//    can request the active scalar.
//
// ****************************************************************************

bool
VolumeGetScalars(const VolumeAttributes &atts, vtkDataSet *ds,
    vtkDataArray *&data, vtkDataArray *&opac)
{
    StackTimer t("VolumeGetScalars");
    const char *ov = atts.GetOpacityVariable().c_str();

    vtkPointData *pd = ds->GetPointData();
    data = VolumeGetScalar(ds, NULL);
    if (data == NULL)
    {
        return false;
    }

    //
    // We are requiring that the return values are freed, so we better add to
    // the reference count.
    //
    data->Register(NULL);

    if (strcmp(ov, "default") == 0)
    {
        //
        // The opacity variable is the same as the coloring variable.  Since
        // we will also free that variable, up the reference count.
        //
        opac = data;
        opac->Register(NULL);
    }
    else
    {
        //
        // The opacity variable is distinct from the coloring variable, so get
        // it.  Unfortunately, we have to create a vtkScalars object from the
        // returned data array.  If we could just return the data array
        // directly, we could get away from all of the memory management we
        // are doing.   KAT -- NOW WE CAN//
        //
        opac = pd->GetArray(ov);
        if (opac == NULL && pd->GetNumberOfArrays() == 1)
        {
            //
            // This can happen when the opacity variable is the same as the
            // coloring variable.  There is a bug with the VTK readers and
            // writers that prevents the active variable from being named.
            // Since the active variable is the coloring variable (which is the
            // opacity variable in this case), we got NULL when we asked for
            // the opacity variable by name.
            //
            opac = pd->GetArray(0);
        }
        if (opac == NULL)
        {
            EXCEPTION0(ImproperUseException);
        }
        opac->Register(NULL);
    }

    return true;
}

// ****************************************************************************
// Method: VolumeGetVariableExtents
//
// Purpose: 
//   Calculates the variable extents, taking the min/max into account.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: 
// Creation:   Thu Dec 18 14:01:06 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
VolumeGetVariableExtents(const VolumeAttributes &atts, vtkDataArray *data,
    float varmin, float varmax, float &vmin, float &vmax, float &vsize)
{
    StackTimer t("VolumeGetVariableExtents");

    // calculate min and max
    VolumeGetRange(data, vmin, vmax);

    // Override with the original extents if appropriate.
    if (atts.GetScaling() != VolumeAttributes::Log)
    {
        vmin = (varmin < vmin ? varmin : vmin);
        vmax = (varmax > vmax ? varmax : vmax);
    }
    else //if (atts.GetScaling() == VolumeAttributes::Log)
    {
        if (varmin > 0)
        {
            float logVarMin = log10(varmin);
            vmin = (logVarMin < vmin ? logVarMin : vmin);
        }
        if (varmax > 0)
        {
            float logVarMax = log10(varmax);
            vmax = (logVarMax < vmax ? logVarMax : vmax);
        }
    }

    // Override with artificial extents if appropriate.
    if (atts.GetUseColorVarMin())
    {
        vmin = atts.GetColorVarMin();
        if (atts.GetScaling() == VolumeAttributes::Log && vmin > 0)
            vmin = log10(vmin);
    }
    if (atts.GetUseColorVarMax())
    {
        vmax = atts.GetColorVarMax();
        if (atts.GetScaling() == VolumeAttributes::Log && vmax > 0)
            vmax = log10(vmax);
    }
    if (vmin >= vmax)
    {
        vmax = vmin + 1.;
    }
    vsize=vmax-vmin;
}

// ****************************************************************************
// Method: VolumeGetOpacityExtents
//
// Purpose: 
//   Calculates the opacity extents, taking the min/max into account.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: 
// Creation:   Thu Dec 18 14:01:06 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
VolumeGetOpacityExtents(const VolumeAttributes &atts, vtkDataArray *opac,
    float &omin, float &omax, float &osize)
{
    StackTimer t("VolumeGetOpacityExtents");

    VolumeGetRange(opac, omin, omax);

    // Override with artificial extents if appropriate.
    if (atts.GetUseOpacityVarMin())
    {
        omin = atts.GetOpacityVarMin();
    }
    if (atts.GetUseOpacityVarMax())
    {
        omax = atts.GetOpacityVarMax();
    }

    // If we set the color var's extents and the opacity variable is the
    // same as the color variable, use the color var's extents.
    if (atts.GetOpacityVariable() == "default")
    {
        if (atts.GetUseColorVarMin())
        {
            omin = atts.GetColorVarMin();
            if (atts.GetScaling() == VolumeAttributes::Log && omin > 0)
                omin = log10(omin);
        }
        if (atts.GetUseColorVarMax())
        {
            omax = atts.GetColorVarMax();
            if (atts.GetScaling() == VolumeAttributes::Log && omax > 0)
                omax = log10(omax);
        }
    }
    if (omin >= omax)
    {
        omax = omin + 1.;
    }
    osize=omax-omin;

}

//
// Macros used in the gradient calculations
//
#define CalculateIndex(GRID, I, J, K) (((K)*ny + (J))*nx + (I))

#define GRADIENT_GET_DIMS \
    int dims[3]; \
    grid->GetDimensions(dims); \
    int nx=dims[0]; \
    int ny=dims[1]; \
    int nz=dims[2];

#define STORE_GRADIENT \
    float mag = sqrtf(gx_i * gx_i + \
                      gy_i * gy_i +\
                      gz_i * gz_i);\
    if(gm != 0)\
        gm[index] = mag;\
    if(gmn != 0)\
        gmn[index] = mag;\
    if (mag > 0)\
    {\
        if(gx != 0)\
            gx[index] = gx_i / mag;\
        if(gy != 0)\
            gy[index] = gy_i / mag;\
        if(gz != 0)\
            gz[index] = gz_i / mag;\
        if (mag > maxmag)\
            maxmag = mag;\
    }

// ****************************************************************************
// Method: VolumeGradient_CenteredDifferences
//
// Purpose: 
//   Calculates the centered-differences gradient.
//
// Arguments:
//   grid : The rectilinear grid that contains the data.
//   opac : The opacity data whose gradient we're calculating.
//   gx   : Optional output array for the X component of the gradient
//   gy   : Optional output array for the Y component of the gradient
//   gz   : Optional output array for the Z component of the gradient
//   gm   : Optional output array for the gradient magnitude
//   gmn  : Optional output array for the normalized gradient magnitude
//
// Returns:    The maximum gradient magnitude.
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   way back
//
// Modifications:
//   Brad Whitlock, Tue Dec 16 14:52:43 PST 2008
//   I extracted this code from the renderer and added a float fast path.
//
// ****************************************************************************

float
VolumeGradient_CenteredDifferences(vtkRectilinearGrid  *grid, vtkDataArray *opac,
    float *gx, float *gy, float *gz, float *gm, float *gmn, float ghostval)
{
    StackTimer t2("CenteredDifferences gradient");

    GRADIENT_GET_DIMS

    vtkDataArray *xc = grid->GetXCoordinates();
    vtkDataArray *yc = grid->GetYCoordinates();
    vtkDataArray *zc = grid->GetZCoordinates();

    float maxmag = 0.f;
    if(opac->GetDataType() == VTK_FLOAT)
    {
        // Float fast path
        int index = 0;
        const float *fopac = (const float *)opac->GetVoidPointer(0);
        for (int k=0; k<nz; k++)
        {
            for (int j=0; j<ny; j++)
            {
                for (int i=0; i<nx; i++, index++)
                {
                    float gx_i, gy_i, gz_i;

                    if (i==0 || (i<nx-1 && fopac[ CalculateIndex(grid,i-1,j  ,k  )] < NO_DATA_VALUE))
                        gx_i = (fopac[CalculateIndex(grid,i+1,j  ,k  )]-fopac[CalculateIndex(grid,i  ,j  ,k  )])/(xc->GetTuple1(i+1)-xc->GetTuple1(i));
                    else if (i==nx-1 || (i>0 && fopac[CalculateIndex(grid,i+1,j  ,k  )] < NO_DATA_VALUE))
                        gx_i = (fopac[CalculateIndex(grid,i  ,j  ,k  )]-fopac[CalculateIndex(grid,i-1,j  ,k  )])/(xc->GetTuple1(i)-xc->GetTuple1(i-1));
                    else
                        gx_i = (fopac[CalculateIndex(grid,i+1,j  ,k  )]-fopac[CalculateIndex(grid,i-1,j  ,k  )])/(xc->GetTuple1(i+1)-xc->GetTuple1(i-1));

                    if (j==0 || (j<ny-1 && fopac[CalculateIndex(grid,i  ,j-1,k  )] < NO_DATA_VALUE))
                        gy_i = (fopac[CalculateIndex(grid,i  ,j+1,k  )]-fopac[CalculateIndex(grid,i  ,j  ,k  )])/(yc->GetTuple1(j+1)-yc->GetTuple1(j ));
                    else if (j==ny-1 || (j>0 && fopac[CalculateIndex(grid,i  ,j+1,k  )] < NO_DATA_VALUE))
                        gy_i = (fopac[CalculateIndex(grid,i  ,j  ,k  )]-fopac[CalculateIndex(grid,i  ,j-1,k  )])/(yc->GetTuple1(j)-yc->GetTuple1(j-1));
                    else
                        gy_i = (fopac[CalculateIndex(grid,i  ,j+1,k  )]-fopac[CalculateIndex(grid,i  ,j-1,k  )])/(yc->GetTuple1(j+1)-yc->GetTuple1(j-1));

                    if (k==0 || (k<nz-1 && fopac[CalculateIndex(grid,i  ,j ,k-1)] < NO_DATA_VALUE))
                        gz_i = (fopac[CalculateIndex(grid,i  ,j  ,k+1)]-fopac[CalculateIndex(grid,i  ,j  ,k  )])/(zc->GetTuple1(k+1)-zc->GetTuple1(k));
                    else if (k==nz-1 || (k>0 && fopac[CalculateIndex(grid,i  ,j ,k+1)] < NO_DATA_VALUE))
                        gz_i = (fopac[CalculateIndex(grid,i  ,j  ,k  )]-fopac[CalculateIndex(grid,i  ,j  ,k-1)])/(zc->GetTuple1(k)-zc->GetTuple1(k-1));
                    else
                        gz_i = (fopac[CalculateIndex(grid,i  ,j  ,k+1)]-fopac[CalculateIndex(grid,i  ,j  ,k-1)])/(zc->GetTuple1(k+1)-zc->GetTuple1(k-1));

                    STORE_GRADIENT
                }
            }
        }
    }
    else
    {
        int index = 0;
        for (int k=0; k<nz; k++)
        {
            for (int j=0; j<ny; j++)
            {
                for (int i=0; i<nx; i++, index++)
                {
                    float gx_i, gy_i, gz_i;

                    if (i==0 || (i<nx-1 && opac->GetTuple1( CalculateIndex(grid,i-1,j  ,k  )) < NO_DATA_VALUE))
                        gx_i = (opac->GetTuple1(CalculateIndex(grid,i+1,j  ,k  ))-opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k  )))/(xc->GetTuple1(i+1)-xc->GetTuple1(i));
                    else if (i==nx-1 || (i>0 && opac->GetTuple1(CalculateIndex(grid,i+1,j  ,k  )) < NO_DATA_VALUE))
                        gx_i = (opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k  ))-opac->GetTuple1(CalculateIndex(grid,i-1,j  ,k  )))/(xc->GetTuple1(i)-xc->GetTuple1(i-1));
                    else
                        gx_i = (opac->GetTuple1(CalculateIndex(grid,i+1,j  ,k  ))-opac->GetTuple1(CalculateIndex(grid,i-1,j  ,k  )))/(xc->GetTuple1(i+1)-xc->GetTuple1(i-1));

                    if (j==0 || (j<ny-1 && opac->GetTuple1(CalculateIndex(grid,i  ,j-1,k  )) < NO_DATA_VALUE))
                        gy_i = (opac->GetTuple1(CalculateIndex(grid,i  ,j+1,k  ))-opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k  )))/(yc->GetTuple1(j+1)-yc->GetTuple1(j ));
                    else if (j==ny-1 || (j>0 && opac->GetTuple1(CalculateIndex(grid,i  ,j+1,k  )) < NO_DATA_VALUE))
                        gy_i = (opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k  ))-opac->GetTuple1(CalculateIndex(grid,i  ,j-1,k  )))/(yc->GetTuple1(j)-yc->GetTuple1(j-1));
                    else
                        gy_i = (opac->GetTuple1(CalculateIndex(grid,i  ,j+1,k  ))-opac->GetTuple1(CalculateIndex(grid,i  ,j-1,k  )))/(yc->GetTuple1(j+1)-yc->GetTuple1(j-1));

                    if (k==0 || (k<nz-1 && opac->GetTuple1(CalculateIndex(grid,i  ,j ,k-1)) < NO_DATA_VALUE))
                        gz_i = (opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k+1))-opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k  )))/(zc->GetTuple1(k+1)-zc->GetTuple1(k));
                    else if (k==nz-1 || (k>0 && opac->GetTuple1(CalculateIndex(grid,i  ,j ,k+1)) < NO_DATA_VALUE))
                        gz_i = (opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k  ))-opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k-1)))/(zc->GetTuple1(k)-zc->GetTuple1(k-1));
                    else
                        gz_i = (opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k+1))-opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k-1)))/(zc->GetTuple1(k+1)-zc->GetTuple1(k-1));

                    STORE_GRADIENT
                }
            }
        }
    }
    return maxmag;
}

// ****************************************************************************
// Method: VolumeGradient_Sobel
//
// Purpose: 
//   Calculates the Sobel gradient.
//
// Arguments:
//   grid : The rectilinear grid that contains the data.
//   opac : The opacity data whose gradient we're calculating.
//   gx   : Optional output array for the X component of the gradient
//   gy   : Optional output array for the Y component of the gradient
//   gz   : Optional output array for the Z component of the gradient
//   gm   : Optional output array for the gradient magnitude
//   gmn  : Optional output array for the normalized gradient magnitude
//
// Returns:    The maximum gradient magnitude.
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   way back
//
// Modifications:
//   Brad Whitlock, Tue Dec 16 14:52:43 PST 2008
//   I extracted this code from the renderer and optimized it to produce a 
//   6.5x speedup for the float case.
//
// ****************************************************************************

float
VolumeGradient_Sobel(vtkRectilinearGrid  *grid, vtkDataArray *opac,
    float *gx, float *gy, float *gz, float *gm, float *gmn, float ghostval)
{
    static const float Mx[3][3][3] = {{{-2, -3, -2}, {-3, -6, -3}, {-2, -3, -2}},
                                      {{ 0,  0,  0}, { 0,  0,  0}, { 0,  0,  0}},
                                      {{ 2,  3,  2}, { 3,  6,  3}, { 2,  3,  2}}};

    static const float My[3][3][3] = {{{-2, -3, -2}, { 0,  0,  0}, { 2,  3,  2}},
                                      {{-3, -6, -3}, { 0,  0,  0}, { 3,  6,  3}},
                                      {{-2, -3, -2}, { 0,  0,  0}, { 2,  3,  2}}};

    static const float Mz[3][3][3] = {{{-2,  0,  2}, {-3,  0,  3}, {-2,  0,  2}},
                                      {{-3,  0,  3}, {-6,  0,  6}, {-3,  0,  3}},
                                      {{-2,  0,  2}, {-3,  0,  3}, {-2,  0,  2}}};
    StackTimer t2("Sobel gradient");

    GRADIENT_GET_DIMS

    float maxmag = 0.;
    if(opac->GetDataType() == VTK_FLOAT)
    {
        // Float fast path
        const float *fopac = (const float *)opac->GetVoidPointer(0);
        int index = 0;
        for (int k=0; k<nz; k++)
        {
            for (int j=0; j<ny; j++)
            {
                for (int i=0; i<nx; i++, index++)
                {
                    float gx_i = 0.f;
                    float gy_i = 0.f;
                    float gz_i = 0.f;

                    // Sample over the Sobel kernel. We build indexing into 
                    // the loops in order to reduce multiplies.
                    for (int c=0; c<3; c++)
                    {
                        int kk = k-1+c;
                        kk = MAX(0, MIN(nz-1, kk));
                        int kny = kk * ny;
                        for (int b=0; b<3; b++)
                        {
                            int jj = j-1+b;
                            jj = MAX(0, MIN(ny-1, jj));
                            int row = (kny + jj) * nx;

                            for (int a=0; a<3; a++)
                            {
                                int ii = i-1+a;
                                ii = MAX(0, MIN(nx-1, ii));

                                float val = fopac[row + ii];
                                if (val < NO_DATA_VALUE)
                                    val = ghostval;

                                gx_i += Mx[a][b][c] * val;
                                gy_i += My[a][b][c] * val;
                                gz_i += Mz[a][b][c] * val;
                            }
                        }
                    }

                    STORE_GRADIENT
                }
            }
        }
    }
    else 
    {
        int index = 0;
        for (int k=0; k<nz; k++)
        {
            for (int j=0; j<ny; j++)
            {
                for (int i=0; i<nx; i++,index++)
                {
                    float gx_i = 0.f;
                    float gy_i = 0.f;
                    float gz_i = 0.f;

                    // Sample over the Sobel kernel. We build indexing into 
                    // the loops in order to reduce multiplies.
                    for (int c=0; c<3; c++)
                    {
                        int kk = k-1+c;
                        kk = MAX(0, MIN(nz-1, kk));
                        int kny = kk * ny;
                        for (int b=0; b<3; b++)
                        {
                            int jj = j-1+b;
                            jj = MAX(0, MIN(ny-1, jj));
                            int row = (kny + jj) * nx;

                            for (int a=0; a<3; a++)
                            {
                                int ii = i-1+a;
                                ii = MAX(0, MIN(nx-1, ii));

                                float val = opac->GetTuple1(row + ii);
                                if (val < NO_DATA_VALUE)
                                    val = ghostval;

                                gx_i += Mx[a][b][c] * val;
                                gy_i += My[a][b][c] * val;
                                gz_i += Mz[a][b][c] * val;
                            }
                        }
                    }

                    STORE_GRADIENT
                }
            }
        }
    }

    return maxmag;
}

// ****************************************************************************
// Method: Unnormalized_Kernel_Gradient
//
// Purpose: 
//   calculates the gradient of the sph kernel
//
// Arguments:
//   r   : radius vector between particles
//   r12 : distance between particles
//   h   : kernel size h
//
// Returns:    gradient of the kernel
//
// Programmer: Allen Harvey
// Creation:   Mon Jan 30 10:31:13 PST 2012
//
// Modifications:
//   
// ****************************************************************************

static void
Unnormalized_Kernel_Gradient(int dim, double r, double *r12, double h,
    double *grad)
{
    double s = 0;
    double temp = 0;

    s = abs(r/h);
// NORMALIZATION IS SET FOR 1-DIMENSION
    if( (s>0.0) && (s<=1.0) )
    {
        temp = (-3.0*s + 2.25*s*s)/h;
        grad[0] = temp * r12[0]/r;
        grad[1] = temp * r12[1]/r;
        grad[2] = temp * r12[2]/r;
    }
    else if( (s>1.0) && (s<2.0) )
    {
        temp = -(3.0*(2.0-s)*(2.0-s))/(4.0*h);
        grad[0] = temp * r12[0]/r;
        grad[1] = temp * r12[1]/r;
        grad[2] = temp * r12[2]/r;
    }
    else
    {
        grad[0] = 0.0;
        grad[1] = 0.0;
        grad[2] = 0.0;
    }
}

static void 
GetConnectedVertices(vtkDataSet* mesh, int seed, vtkSmartPointer<vtkIdList> connectedVertices)
{
    //get all cells that vertex 'seed' is a part of
    vtkSmartPointer<vtkIdList> cellIdList = vtkSmartPointer<vtkIdList>::New();
    mesh->GetPointCells(seed, cellIdList);

    //loop through all the cells that use the seed point
    for(vtkIdType i = 0; i < cellIdList->GetNumberOfIds(); i++)
    {
        vtkCell* cell = mesh->GetCell(cellIdList->GetId(i));

        //if the cell doesn't have any edges, it is a line
        if(cell->GetNumberOfEdges() <= 0)
        {
            continue;
        }

        for(vtkIdType e = 0; e < cell->GetNumberOfEdges(); e++)
        {
            vtkCell* edge = cell->GetEdge(e);

            vtkIdList* pointIdList = edge->GetPointIds();

            if(pointIdList->GetId(0) == seed || pointIdList->GetId(1) == seed)
            {
                if(pointIdList->GetId(0) == seed)
                {
                    connectedVertices->InsertUniqueId(pointIdList->GetId(1));
                }
                else
                {
                    connectedVertices->InsertUniqueId(pointIdList->GetId(0));
                }
            }
        } //end for
    }
}

// ****************************************************************************
// Method: VolumeCalculateGradient_SPH
//
// Purpose: 
//   Calculates the gradient by performing Delauney Triangulization followed
//       by applying an SPH Kernel.
//
// Arguments:
//   ds   : The grid that contains the data.
//   opac : The opacity data whose gradient we're calculating.
//   gx   : Optional output array for the X component of the gradient
//   gy   : Optional output array for the Y component of the gradient
//   gz   : Optional output array for the Z component of the gradient
//   gm   : Optional output array for the gradient magnitude
//   gmn  : Optional output array for the normalized gradient magnitude
//
// Returns:    The maximum gradient magnitude.
//
// Note:       
//
// Programmer: Allen Harvey
// Creation:   Mon Jan 30 10:33:39 PST 2012
//
// Modifications:
//
// ****************************************************************************

float
VolumeCalculateGradient_SPH(vtkDataSet *ds, vtkDataArray *opac,
    float *gx, float *gy, float *gz, float *gm, float *gmn, float *hs, bool calcHS, float ghostval)
{
    StackTimer t2("SPH gradient");

    float maxmag = 0.f;

    const float *fopac = (const float *)opac->GetVoidPointer(0);

    //Apply a delauney operator on the data
    //code taken from avtDelaunayFilter.C, ExecuteData Method
    vtkDelaunay3D *d3 = NULL;

    vtkDataSet *outDS = NULL;
    int dimension = 3;  //For now, support is limited to 3D only
    double p[3], r[3], grad[3];
    double h = 0.0, hmax = 0.0, radius = 0.0;
    p[0] = p[1] = p[2] = 0.0;
    r[0] = r[1] = r[2] = 0.0;
    grad[0] = grad[1] = grad[2] = 0.0;

    d3 = vtkDelaunay3D::New();
    d3->SetInput(ds);
    outDS = vtkUnstructuredGrid::New();
    d3->SetOutput(outDS);

    outDS->Update();

    if (d3)
        d3->Delete();

    //Loop over all the points.  Collect the points around them (points around points).
    //  build an SPH kernel around them.
    vtkSmartPointer<vtkIdList> connectedVertices = vtkSmartPointer<vtkIdList>::New();
    for ( int index = 0; index < outDS->GetNumberOfPoints(); index++ )
    {
        connectedVertices->Reset();

        GetConnectedVertices(outDS, index, connectedVertices);

        //If the user provides a compact support variable, use it for 'h', otherwise calculate an 'h'.
        if (calcHS)
        {
            //find the kernel size (i.e. the longest distance)
            outDS->GetPoint(index,p);
            hmax = 0.0;
            for(vtkIdType i = 0; i < connectedVertices->GetNumberOfIds(); i++)
            {
                outDS->GetPoint(connectedVertices->GetId(i), r);
                r[0] = p[0] - r[0];
                r[1] = p[1] - r[1];
                r[2] = p[2] - r[2];
                h = r[0]*r[0]+r[1]*r[1]+r[2]*r[2];
                h = sqrt(h);
                hmax = MAX(h,hmax);
                //cout<<"hmax is now: "<<hmax<<endl;
            }
            //calculate the gradient
            h = hmax/2.0;
            if(hs != NULL)
                hs[index] = hmax;
        }
        else if(hs != NULL)
        {
            //Use the prescribed variable
            h = hs[index];
        }

        float temp = PI*h*h*h;
        float gx_i = 0.f;
        float gy_i = 0.f;
        float gz_i = 0.f;

        for(vtkIdType i = 0; i < connectedVertices->GetNumberOfIds(); i++)
        {
            outDS->GetPoint(connectedVertices->GetId(i), r);
            r[0] = p[0] - r[0];
            r[1] = p[1] - r[1];
            r[2] = p[2] - r[2];
            radius = r[0]*r[0]+r[1]*r[1]+r[2]*r[2];
            radius = sqrt(radius);
            Unnormalized_Kernel_Gradient(dimension, radius, r, h, grad);
            grad[0] = grad[0]/temp;
            grad[1] = grad[1]/temp;
            grad[2] = grad[2]/temp;

            gx_i = gx_i + grad[0]*fopac[i];
            gy_i = gy_i + grad[1]*fopac[i];
            gz_i = gz_i + grad[2]*fopac[i];
        }
        STORE_GRADIENT
    }

    connectedVertices->Reset();

    if (maxmag > 0 && gmn != 0)
    {
        vtkIdType nels = outDS->GetNumberOfPoints();
        for (vtkIdType n=0; n<nels; n++)
            gmn[n] /= maxmag;
    }

    if(outDS)
        outDS->Delete();

    return maxmag;
}

// ****************************************************************************
// Method: VolumeCalculateGradient
//
// Purpose: 
//   Calculates gradient arrays.
//
// Arguments:
//   grid : The rectilinear grid that contains the data.
//   opac : The opacity data whose gradient we're calculating.
//   gx   : Optional output array for the X component of the gradient
//   gy   : Optional output array for the Y component of the gradient
//   gz   : Optional output array for the Z component of the gradient
//   gm   : Optional output array for the gradient magnitude
//   gmn  : Optional output array for the normalized gradient magnitude
//   ghostval : The ghost value.
//
// Returns: the maximum gradient magnitude in the data
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 17 16:06:14 PST 2008
//
// Modifications:
//   Jeremy Meredith, Tue Jan  5 15:51:03 EST 2010
//   Had it return the maximum gradient magnitude.
//
// ****************************************************************************

float
VolumeCalculateGradient(const VolumeAttributes &atts, 
    vtkRectilinearGrid  *grid, vtkDataArray *opac,
    float *gx, float *gy, float *gz, float *gm, float *gmn, float ghostval)
{
    StackTimer t("VolumeCalculateGradient");

    GRADIENT_GET_DIMS
    int nels=nx*ny*nz;

    float maxmag = 0;
    if (atts.GetGradientType() == VolumeAttributes::CenteredDifferences)
    {
        maxmag = VolumeGradient_CenteredDifferences(grid, opac, gx, gy, gz, 
                                                    gm, gmn, ghostval);
    }
    else //(atts.GetGradientType() == VolumeAttributes::SobelOperator)
    {
        maxmag = VolumeGradient_Sobel(grid, opac, gx, gy, gz, 
                                      gm, gmn, ghostval);
    }

    if (maxmag > 0 && gmn != 0)
    {
        for (int n=0; n<nels; n++)
            gmn[n] /= maxmag;
    }

    return maxmag;
}

// ****************************************************************************
// Method: VolumeHistograms
//
// Purpose: 
//   Computes a 1D and 2D histogram of gradient magnitude vs. var and stores the 
//   results in a 2D float array whose values are in [0.,1.].
//
// Arguments:
//   data      : The variable data that we're histogramming.
//   gm        : The gradient magnitude.
//   hist      : The destination array for the histogram.
//   hist_size : The size of one dimension of the histogram array.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 16 14:48:44 PST 2008
//
// Modifications:
//  Cyrus Harrison, Tue Apr  5 09:07:22 PDT 2011
//  Guard against invalid mem access when the var range or grad range is zero.
//
//  Kathleen Biagas, Thu May 31 10:12:48 PDT 2012
//  Add continue's to for-loop for s out of var_min/var_max range when 
//  populating histograms in the non-float case.
//  
// ****************************************************************************

void
VolumeHistograms(const VolumeAttributes &atts, 
    vtkDataArray *data, vtkDataArray *gm, 
    float *hist, float *hist2, int hist_size)
{
    const char *mName = "VolumeHistogram2D: ";
    StackTimer t("VolumeHistogram2D");

    // Get the range for the data var.
    float var_min, var_max;
    VolumeGetRange(data, var_min, var_max);
    if(atts.GetUseColorVarMin())
        var_min = atts.GetColorVarMin();
    if(atts.GetUseColorVarMax())
        var_max = atts.GetColorVarMax();
    float var_diff = var_max - var_min;
    debug5 << mName << "Var range: " << var_min << ", " << var_max << ", diff=" << var_diff << endl;

    // Get the range for the gradient magnitude
    float grad_min, grad_max;
    VolumeGetRange(gm, grad_min, grad_max);
    float grad_diff = grad_max - grad_min;
    debug5 << mName << "GM range: " << grad_min << ", " << grad_max << ", diff=" << grad_diff << endl;

    // Initialize the output arrays.
    memset(hist2, 0, sizeof(float) * hist_size * hist_size);
    memset(hist, 0, sizeof(float) * hist_size);

    // Populate histograms
    int N = data->GetNumberOfTuples();
    float hist_max = 0.;
    float hist2_max = 0.;
    float s_scale = var_diff  !=0 ? (hist_size - 1) / var_diff  : 0.0;
    float m_scale = grad_diff !=0 ? (hist_size - 1) / grad_diff : 0.0;
    if(data->GetDataType() == VTK_FLOAT &&
       gm->GetDataType() == VTK_FLOAT)
    {
        const float *scalar = (const float *)data->GetVoidPointer(0);
        const float *end = scalar + N;
        const float *magnitude = (const float *)gm->GetVoidPointer(0);
        while(scalar < end)
        {
            float s = *scalar++;
            float m = *magnitude++;
            if(s < var_min)
                continue;
            if(s > var_max)
                continue;

            int scalar_index = (int)(s_scale * (s - var_min));
            int mag_index    = (int)(m_scale * (m - grad_min));
            int hindex = (mag_index * hist_size) + scalar_index;

            hist[scalar_index] += 1.;
            if(hist[scalar_index] > hist_max)
                hist_max = hist[scalar_index];

            hist2[hindex] += 1.;
            if(hist2[hindex] > hist2_max)
                hist2_max = hist2[hindex];
        }
    }
    else
    {
        for(int index = 0; index < N; ++index)
        {
            float s = data->GetTuple1(index);
            if(s < NO_DATA_VALUE)
                continue;
            if(s < var_min)
                continue;
            if(s > var_max)
                continue;

            int scalar_index = (int)(s_scale * (s - var_min));
            hist[scalar_index] += 1.;
            if(hist[scalar_index] > hist_max)
                hist_max = hist[scalar_index];

            int mag_index    = (int)(m_scale * (gm->GetTuple1(index)   - grad_min));
            int hindex = (mag_index * hist_size) + scalar_index;
            hist2[hindex] += 1.;
            if(hist2[hindex] > hist2_max)
                hist2_max = hist2[hindex];
        }
    }

    // Go through the 2D histogram data and scale it to [.1,1.] so we can
    // use the results as a decent GL texture.
    if(hist2_max > 0.)
    {
        int hist_size2 = hist_size * hist_size;
        float h_scale = 0.9 / hist2_max;
        for (int index = 0; index < hist_size2; ++index)
        {
            if(hist2[index] > 0.)
                hist2[index] = hist2[index] * h_scale + 0.1;
        }
    }
    // Normalize the 1D histogram data.
    if(hist_max > 0.)
    {
        float h_scale = 1. / hist_max;
        for (int index = 0; index < hist_size; ++index)
            hist[index] *= h_scale;
    }
}
