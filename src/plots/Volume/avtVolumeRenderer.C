// ************************************************************************* //
//                             avtVolumeRenderer.C                           //
// ************************************************************************* //

#include "avtVolumeRenderer.h"

#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <math.h>
#include <float.h>

#include <avtCallback.h>
#include <avtOpenGLSplattingVolumeRenderer.h>
#include <avtMesaSplattingVolumeRenderer.h>
#include <avtOpenGL3DTextureVolumeRenderer.h>
#include <avtMesa3DTextureVolumeRenderer.h>

#include <ImproperUseException.h>

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

// private prototype -- I can't believe VTK doesn't already provide this:
static int CalculateIndex(vtkRectilinearGrid*, const int,const int,const int);


// ****************************************************************************
//  Constructor:  avtVolumeRenderer::avtVolumeRenderer
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 30 11:44:21 PDT 2003
//    Pulled out the reference to alphatex.  It belonged in the subclass.
//
// ****************************************************************************
avtVolumeRenderer::avtVolumeRenderer()
{
    initialized = false;

    rendererImplementation = NULL;
    currentRendererIsValid = false;

    gx  = NULL;
    gy  = NULL;
    gz  = NULL;
    gm  = NULL;
    gmn = NULL;
}

// ****************************************************************************
//  Destructor:  avtVolumeRenderer::~avtVolumeRenderer
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 30 11:44:21 PDT 2003
//    Pulled out the reference to alphatex.  It belonged in the subclass.
//    Added a call to ReleaseGraphicsResources.
//
// ****************************************************************************
avtVolumeRenderer::~avtVolumeRenderer()
{
    ReleaseGraphicsResources();
    delete[] gx;
    delete[] gy;
    delete[] gz;
    delete[] gm;
    delete[] gmn;
}

// ****************************************************************************
//  Method:  
//
//  Purpose:
//    
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  1, 2003
//
// ****************************************************************************
void
avtVolumeRenderer::ReleaseGraphicsResources()
{
    if (rendererImplementation)
    {
        VTKRen->GetRenderWindow()->MakeCurrent();
        delete rendererImplementation;
        rendererImplementation = NULL;
    }
}

// ****************************************************************************
//  Method: avtVolumeRenderer::New
//
//  Purpose:
//      A static method that creates the correct type of renderer based on
//      whether we should use OpenGL or Mesa.
//
//  Returns:    A renderer that is of type derived from this class.
//
//  Programmer: Hank Childs
//  Creation:   April 24, 2002
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 30 11:47:16 PDT 2003
//    Renamed the renderers to contain "Splatting" in their name.
//
// ****************************************************************************
avtVolumeRenderer *
avtVolumeRenderer::New(void)
{
    return new avtVolumeRenderer;
}
 
// ****************************************************************************
//  Method:  avtVolumeRenderer::Render
//
//  Purpose:
//    Set up things necessary to call the renderer implentation.  Make a new
//    implementation object if things have changed.
//
//  Arguments:
//    ds         the dataset to render
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  1, 2003
//
// ****************************************************************************
void
avtVolumeRenderer::Render(vtkDataSet *ds)
{
    if (!currentRendererIsValid || !rendererImplementation)
    {
        if (rendererImplementation)
            delete rendererImplementation;

        if (avtCallback::GetNowinMode())
        {
            if (atts.GetRendererType() == VolumeAttributes::Splatting)
                rendererImplementation = new avtMesaSplattingVolumeRenderer;
            else // it == VolumeAttributes::Texture3D
                rendererImplementation = new avtMesa3DTextureVolumeRenderer;
        }
        else
        { 
            if (atts.GetRendererType() == VolumeAttributes::Splatting)
                rendererImplementation = new avtOpenGLSplattingVolumeRenderer;
            else // it == VolumeAttributes::Texture3D
                rendererImplementation = new avtOpenGL3DTextureVolumeRenderer;
        }
        currentRendererIsValid = true;
    }

    // Do other initialization
    if (!initialized)
    {
        Initialize(ds);
    }

    // get data set
    vtkRectilinearGrid  *grid = (vtkRectilinearGrid*)ds;
    vtkDataArray *data = NULL;
    vtkDataArray *opac = NULL;
    bool haveScalars = GetScalars(grid, data, opac);

    if (haveScalars)
        rendererImplementation->Render(grid, data, opac, view, atts,
                                       vmin, vmax, vsize,
                                       omin, omax, osize,
                                       gx, gy, gz, gmn);
}


// ****************************************************************************
//  Method:  avtVolumeRenderer::Initialize
//
//  Purpose:
//    Calculate some one-time stuff with respect to the data set
//
//  Arguments:
//    ds      : the data set
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Nov 13 11:31:57 PST 2001
//    Made it use the Sobel gradient operator by default since it looks so 
//    much better.  Fixed the Sobel operator to work with ghost values.
//
//    Hank Childs, Mon Nov 19 08:49:55 PST 2001
//    Used the opacity variable to calculate gradients.
//
//    Hank Childs, Mon Nov 19 15:34:44 PST 2001
//    Use the original variable extents when coloring.
//
//    Hank Childs, Wed Dec 12 10:54:58 PST 2001
//    Allow for variable extents to be artificially set.
//
//    Kathleen Bonnell, Mon Nov 19 16:05:37 PST 2001 
//    VTK 4.0 API changes require use of vtkDataArray in place of
//    vtkScalars for rgrid coordinates. 
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Jeremy Meredith, Tue Sep 30 11:47:41 PDT 2003
//    Only calculate the gradient if lighting is enabled *and* if it does
//    not already exist.  The SetAtts method will delete it if it is invalid.
//
//    Jeremy Meredith, Thu Oct  2 13:15:27 PDT 2003
//    Choose the gradient method based on settings from the user.  Changed
//    the ghost method to avoid ghosts entirely.
//
// ****************************************************************************
void
avtVolumeRenderer::Initialize(vtkDataSet *ds)
{
    // get data set
    vtkRectilinearGrid  *grid = (vtkRectilinearGrid*)ds;
    vtkDataArray        *data = NULL;
    vtkDataArray        *opac = NULL;
    if (!GetScalars(grid, data, opac))
        return;

    int dims[3];
    grid->GetDimensions(dims);

    // calculate min and max
    GetRange(data, vmin, vmax);

    // Override with the original extents if appropriate.
    vmin = (varmin < vmin ? varmin : vmin);
    vmax = (varmax > vmax ? varmax : vmax);

    // Override with artificial extents if appropriate.
    if (atts.GetUseColorVarMin())
    {
        vmin = atts.GetColorVarMin();
    }
    if (atts.GetUseColorVarMax())
    {
        vmax = atts.GetColorVarMax();
    }
    if (vmin >= vmax)
    {
        vmax = vmin + 1.;
    }
    vsize=vmax-vmin;

    GetRange(opac, omin, omax);

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
        }
        if (atts.GetUseColorVarMax())
        {
            omax = atts.GetColorVarMax();
        }
    }
    if (omin >= omax)
    {
        omax = omin + 1.;
    }
    osize=omax-omin;

    // calculate gradient
    if (atts.GetLightingFlag() &&
        !gx) // make sure the gradient was invalidated first
    {
        int nx=dims[0];
        int ny=dims[1];
        int nz=dims[2];
        int nels=nx*ny*nz;
        gx  = new float[nels];
        gy  = new float[nels];
        gz  = new float[nels];
        gm  = new float[nels];
        gmn = new float[nels];

        vtkDataArray *xc = grid->GetXCoordinates();
        vtkDataArray *yc = grid->GetYCoordinates();
        vtkDataArray *zc = grid->GetZCoordinates();

        // Some default values since they are not set from outside yet
        const int ghost = 0;

        float ghostval = (ghost == 1) ? omin-osize : omax+osize;
        float maxmag = 0;
        for (int i=0; i<nx; i++)
        {
            for (int j=0; j<ny; j++)
            {
                for (int k=0; k<nz; k++)
                {
                    int index = CalculateIndex(grid,i,j,k);

                    if (atts.GetGradientType() == VolumeAttributes::CenteredDifferences)
                    {
                        if (ghost != 0)
                        {
                            if (i==0 || (i<nx-1 && opac->GetTuple1( CalculateIndex(grid,i-1,j  ,k  )) < -1e+37))
                                gx[index] = (opac->GetTuple1(CalculateIndex(grid,i+1,j  ,k  )) - ghostval                                         )/(2*(xc->GetTuple1(i+1)-xc->GetTuple1(i)));
                            else if (i==nx-1 || (i>0 && opac->GetTuple1(CalculateIndex(grid,i+1,j  ,k  )) < -1e+37))
                                gx[index] = (ghostval                                          - opac->GetTuple1(CalculateIndex(grid,i-1,j  ,k  )))/(2*(xc->GetTuple1(i)-xc->GetTuple1(i-1)));
                            else
                                gx[index] = (opac->GetTuple1(CalculateIndex(grid,i+1,j  ,k  )) - opac->GetTuple1(CalculateIndex(grid,i-1,j  ,k  )))/(   xc->GetTuple1(i+1)-xc->GetTuple1(i-1));

                            if (j==0 || (j<ny-1 && opac->GetTuple1(CalculateIndex(grid,i  ,j-1,k  )) < -1e+37))
                                gy[index] = (opac->GetTuple1(CalculateIndex(grid,i  ,j+1,k  )) - ghostval                                         )/(2*(yc->GetTuple1(j+1)-yc->GetTuple1(j)));
                            else if (j==ny-1 || (j>0 && opac->GetTuple1(CalculateIndex(grid,i  ,j+1,k  )) < -1e+37))
                                gy[index] = (ghostval                                          - opac->GetTuple1(CalculateIndex(grid,i  ,j-1,k  )))/(2*(yc->GetTuple1(j)-yc->GetTuple1(j-1)));
                            else
                                gy[index] = (opac->GetTuple1(CalculateIndex(grid,i  ,j+1,k  )) - opac->GetTuple1(CalculateIndex(grid,i  ,j-1,k  )))/(   yc->GetTuple1(j+1)-yc->GetTuple1(j-1));

                            if (k==0 || (k<nz-1 && opac->GetTuple1(CalculateIndex(grid,i  ,j ,k-1)) < -1e+37))
                                gz[index] = (opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k+1)) - ghostval                                         )/(2*(zc->GetTuple1(k+1)-zc->GetTuple1(k)));
                            else if (k==nz-1 || (k>0 && opac->GetTuple1(CalculateIndex(grid,i  ,j ,k+1)) < -1e+37))
                                gz[index] = (ghostval                                          - opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k-1)))/(2*(zc->GetTuple1(k)-zc->GetTuple1(k-1)));
                            else
                                gz[index] = (opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k+1)) - opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k-1)))/(   zc->GetTuple1(k+1)-zc->GetTuple1(k-1));
                        }
                        else
                        {
                            if (i==0 || opac->GetTuple1(CalculateIndex(grid,i-1,j  ,k  )) < -1e+37)
                                gx[index] = (opac->GetTuple1(CalculateIndex(grid,i+1,j  ,k  ))-opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k  )))/(xc->GetTuple1(i+1)-xc->GetTuple1(i));
                            else if (i==nx-1 || opac->GetTuple1(CalculateIndex(grid,i+1,j  ,k  )) < -1e+37)
                                gx[index] = (opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k  ))-opac->GetTuple1(CalculateIndex(grid,i-1,j  ,k  )))/(xc->GetTuple1(i)-xc->GetTuple1(i-1));
                            else
                                gx[index] = (opac->GetTuple1(CalculateIndex(grid,i+1,j  ,k  ))-opac->GetTuple1(CalculateIndex(grid,i-1,j  ,k  )))/(xc->GetTuple1(i+1)-xc->GetTuple1(i-1));

                            if (j==0 || opac->GetTuple1(CalculateIndex(grid,i  ,j-1,k  )) < -1e+37)
                                gy[index] = (opac->GetTuple1(CalculateIndex(grid,i  ,j+1,k  ))-opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k  )))/(yc->GetTuple1(j+1)-yc->GetTuple1(j ));
                            else if (j==ny-1 || opac->GetTuple1(CalculateIndex(grid,i  ,j+1,k  )) < -1e+37)
                                gy[index] = (opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k  ))-opac->GetTuple1(CalculateIndex(grid,i  ,j-1,k  )))/(yc->GetTuple1(j)-yc->GetTuple1(j-1));
                            else
                                gy[index] = (opac->GetTuple1(CalculateIndex(grid,i  ,j+1,k  ))-opac->GetTuple1(CalculateIndex(grid,i  ,j-1,k  )))/(yc->GetTuple1(j+1)-yc->GetTuple1(j-1));

                            if (k==0 || opac->GetTuple1(CalculateIndex(grid,i  ,j ,k-1)) < -1e+37)
                                gz[index] = (opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k+1))-opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k  )))/(zc->GetTuple1(k+1)-zc->GetTuple1(k));
                            else if (k==nz-1 || opac->GetTuple1(CalculateIndex(grid,i  ,j ,k+1)) < -1e+37)
                                gz[index] = (opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k  ))-opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k-1)))/(zc->GetTuple1(k)-zc->GetTuple1(k-1));
                            else
                                gz[index] = (opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k+1))-opac->GetTuple1(CalculateIndex(grid,i  ,j  ,k-1)))/(zc->GetTuple1(k+1)-zc->GetTuple1(k-1));
                        }
                    }
                    else //(atts.GetGradientType() == VolumeAttributes::SobelOperator)
                    {
                        float Mx[3][3][3] = {{{-2, -3, -2}, {-3, -6, -3}, {-2, -3, -2}},
                                             {{ 0,  0,  0}, { 0,  0,  0}, { 0,  0,  0}},
                                             {{ 2,  3,  2}, { 3,  6,  3}, { 2,  3,  2}}};

                        float My[3][3][3] = {{{-2, -3, -2}, { 0,  0,  0}, { 2,  3,  2}},
                                             {{-3, -6, -3}, { 0,  0,  0}, { 3,  6,  3}},
                                             {{-2, -3, -2}, { 0,  0,  0}, { 2,  3,  2}}};

                        float Mz[3][3][3] = {{{-2,  0,  2}, {-3,  0,  3}, {-2,  0,  2}},
                                             {{-3,  0,  3}, {-6,  0,  6}, {-3,  0,  3}},
                                             {{-2,  0,  2}, {-3,  0,  3}, {-2,  0,  2}}};


                        gx[index] = 0;
                        gy[index] = 0;
                        gz[index] = 0;
                        for (int a=-1; a<=1; a++)
                        {
                            for (int b=-1; b<=1; b++)
                            {
                                for (int c=-1; c<=1; c++)
                                {
                                    float val;
                               
                                    int ii = i+a;
                                    int jj = j+b;
                                    int kk = k+c;

                                    if (ghost != 0 && 
                                        (ii < 0 || ii > nx-1 ||
                                         jj < 0 || jj > ny-1 ||
                                         kk < 0 || kk > nz-1))
                                    {
                                        val = ghostval;
                                    }
                                    else
                                    {
                                        ii = MAX(0, MIN(nx-1, ii));
                                        jj = MAX(0, MIN(ny-1, jj));
                                        kk = MAX(0, MIN(nz-1, kk));
                                        val = opac->GetTuple1(CalculateIndex(grid,ii,jj,kk));
                                        if (val < -1e+37)
                                            val = ghostval;
                                    }

                                    gx[index] += Mx[a+1][b+1][c+1] * val;
                                    gy[index] += My[a+1][b+1][c+1] * val;
                                    gz[index] += Mz[a+1][b+1][c+1] * val;
                                }
                            }
                        }
                    }

                    // Normalize the computed gradient
                    float mag = sqrt(gx[index]*gx[index] +
                                     gy[index]*gy[index] +
                                     gz[index]*gz[index]);
                    gm[index] = mag;
                    gmn[index] = mag;
                    if (mag>0)
                    {
                        gx[index] /= mag;
                        gy[index] /= mag;
                        gz[index] /= mag;
                    }

                    if (mag > maxmag)
                        maxmag = mag;
                }
            }
        }
        if (maxmag > 0)
        {
            for (int n=0; n<nels; n++)
                gmn[n] /= maxmag;
        }
    }
    opac->Delete();
    data->Delete();

    initialized = true;
}

// ****************************************************************************
//  Method:  avtVolumeRenderer::SetAtts
//
//  Purpose:
//    Set the attributes
//
//  Arguments:
//    a       : the new attributes
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2001
//
//  Modifications:
//    Hank Childs, Wed Dec 12 11:15:56 PST 2001
//    Re-initialize if the atts change, so new extents can be set.
//
//    Jeremy Meredith, Tue Sep 30 11:48:46 PDT 2003
//    Make sure we need to invalidate the gradient before doing so.
//
//    Jeremy Meredith, Thu Oct  2 13:16:15 PDT 2003
//    Added support for multiple renderer types.
//
// ****************************************************************************
void
avtVolumeRenderer::SetAtts(const AttributeGroup *a)
{
    const VolumeAttributes *newAtts = (const VolumeAttributes*)a;

    bool invalidateGradient = !(atts.GradientWontChange(*newAtts));
    currentRendererIsValid = (atts.GetRendererType() == newAtts->GetRendererType());

    atts = *(const VolumeAttributes*)a;

    // Clean up memory.
    if (invalidateGradient)
    {
        if (gx != NULL)
        {
            delete[] gx;
            gx = NULL;
        }
        if (gy != NULL)
        {
            delete[] gy;
            gy = NULL;
        }
        if (gz != NULL)
        {
            delete[] gz;
            gz = NULL;
        }
        if (gm != NULL)
        {
            delete[] gm;
            gm = NULL;
        }
        if (gmn != NULL)
        {
            delete[] gmn;
            gmn = NULL;
        }
    }

    initialized = false;
}

// ****************************************************************************
//  Method: avtVolumeRenderer::GetScalars
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
// ****************************************************************************
bool
avtVolumeRenderer::GetScalars(vtkDataSet *ds, vtkDataArray *&data,
                                  vtkDataArray *&opac)
{
    const char *ov = atts.GetOpacityVariable().c_str();

    vtkPointData *pd = ds->GetPointData();
    data = pd->GetScalars();
    if (data == NULL)  
    {
        //
        // The data is not set up as the active scalars.  Try to guess what
        // it should be.
        //
        for (int i = 0 ; i < pd->GetNumberOfArrays() ; i++)
        {
            vtkDataArray *arr = pd->GetArray(i);
            if (strcmp(arr->GetName(), ov) == 0)
            {
                if (pd->GetNumberOfArrays() > 1)
                {
                    continue;
                }
            }
            data = arr;
        }
    }
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
        // are doing.   KAT -- NOW WE CAN!
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
//  Method: avtVolumeRenderer::GetRange
//
//  Purpose:
//      Determines the range for a scalar variable.
//
//  Arguments:
//      s       The scalar variable.  This variable may have some dummy values
//              (like -1e+37, etc).
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
// ****************************************************************************

void
avtVolumeRenderer::GetRange(vtkDataArray *s, float &min, float &max)
{
    min = +FLT_MAX;
    max = -FLT_MAX;
    int nScalars = s->GetNumberOfTuples();
    for (int i = 0 ; i < nScalars ; i++)
    {
        float v=s->GetTuple1(i);
        if (v < -1e+37)
            continue;
        if (v < min)
            min = v;
        if (v > max)
            max = v;
    }
}

// ****************************************************************************
//  Function:  CalculateIndex
//
//  Purpose:
//    Call ComputePointId for a rectilinear grid.  Prevents user from having
//    to use an array.
//
//  Arguments:
//    grid    : the rectilinear grid
//    i,j,k   : the indices
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2001
//
// ****************************************************************************
static int
CalculateIndex(vtkRectilinearGrid *grid, const int i,const int j,const int k)
{
    static int ijk[3];
    ijk[0] = i;
    ijk[1] = j;
    ijk[2] = k;
    return grid->ComputePointId(ijk);
}

