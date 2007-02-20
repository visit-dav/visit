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
//                          avtSamplePointExtractor.C                        //
// ************************************************************************* //

#include <avtSamplePointExtractor.h>

#include <float.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkHexahedron.h>
#include <vtkPixel.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPyramid.h>
#include <vtkQuad.h>
#include <vtkTetra.h>
#include <vtkTriangle.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVoxel.h>
#include <vtkWedge.h>

#include <avtCellList.h>
#include <avtDatasetExaminer.h>
#include <avtHexahedronExtractor.h>
#include <avtMassVoxelExtractor.h>
#include <avtParallel.h>
#include <avtPointExtractor.h>
#include <avtPyramidExtractor.h>
#include <avtRayFunction.h>
#include <avtSamplePoints.h>
#include <avtTetrahedronExtractor.h>
#include <avtVolume.h>
#include <avtWedgeExtractor.h>

#include <DebugStream.h>
#include <InvalidCellTypeException.h>
#include <TimingsManager.h>


// ****************************************************************************
//  Method: avtSamplePointExtractor constructor
//
//  Arguments:
//      w       The width.
//      h       The height.
//      d       The depth.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//     
//  Modifications:
//
//    Hank Childs, Thu Nov 15 15:39:48 PST 2001
//    Moved construction of cell list to Execute to account new limitations of
//    sample points involving multiple variables.
//
//    Hank Childs, Tue Jan  1 10:01:20 PST 2002
//    Initialized sendCells.
//
//    Hank Childs, Sun Dec 14 11:07:56 PST 2003
//    Initialized massVoxelExtractor.
//
//    Hank Childs, Fri Nov 19 13:57:02 PST 2004
//    Initialized rectilinearGridsAreInWorldSpace.
//
//    Hank Childs, Fri Dec 10 09:59:57 PST 2004
//    Initialized shouldDoTiling.
//
//    Hank Childs, Wed Feb  2 08:56:00 PST 2005
//    Initialize modeIs3D.
//
//    Hank Childs, Sun Dec  4 19:12:42 PST 2005
//    Initialize kernelBasedSampling.
//
//    Hank Childs, Tue Jan 24 16:42:40 PST 2006
//    Added point extractor.
//
// ****************************************************************************

avtSamplePointExtractor::avtSamplePointExtractor(int w, int h, int d)
{
    width  = w;
    height = h;
    depth  = d;

    currentNode = 0;
    totalNodes  = 0;

    hexExtractor        = NULL;
    massVoxelExtractor  = NULL;
    pointExtractor      = NULL;
    pyramidExtractor    = NULL;
    tetExtractor        = NULL;
    wedgeExtractor      = NULL;

#ifdef PARALLEL
    sendCells        = true;
#else
    sendCells        = false;
#endif
    rayfoo           = NULL;

    rectilinearGridsAreInWorldSpace = false;
    aspect = 1.;

    shouldDoTiling = false;

    modeIs3D = true;
    SetKernelBasedSampling(false);
}


// ****************************************************************************
//  Method: avtSamplePointExtractor destructor
//
//  Programmer: Hank Childs
//  Creation:   December 8, 2000
//      
//  Modifications:
//
//    Hank Childs, Sun Dec 14 11:07:56 PST 2003
//    Deleted massVoxelExtractor.
//
//    Hank Childs, Tue Jan 24 16:42:40 PST 2006
//    Deleted pointExtractor.
//
// ****************************************************************************

avtSamplePointExtractor::~avtSamplePointExtractor()
{
    if (hexExtractor != NULL)
    {
        delete hexExtractor;
        hexExtractor = NULL;
    }
    if (massVoxelExtractor != NULL)
    {
        delete massVoxelExtractor;
        massVoxelExtractor = NULL;
    }
    if (tetExtractor != NULL)
    {
        delete tetExtractor;
        tetExtractor = NULL;
    }
    if (wedgeExtractor != NULL)
    {
        delete wedgeExtractor;
        wedgeExtractor = NULL;
    }
    if (pointExtractor != NULL)
    {
        delete pointExtractor;
        pointExtractor = NULL;
    }
    if (pyramidExtractor != NULL)
    {
        delete pyramidExtractor;
        pyramidExtractor = NULL;
    }
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::SetKernelBasedSampling
//  
//  Purpose:
//      Sets whether or not we are doing kernel-based sampling.
//
//  Programmer: Hank Childs
//  Creation:   January 16, 2006
//
// ****************************************************************************

void
avtSamplePointExtractor::SetKernelBasedSampling(bool val)
{
    kernelBasedSampling = val;
    avtRay::SetKernelBasedSampling(kernelBasedSampling);
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::SetRectilinearGridsAreInWorldSpace
//
//  Purpose:
//      Tells this object that any input rectilinear grids are in world space,
//      not image space.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 12:14:04 EST 2007
//    Set rectilinearGridsAreInWorldSpace based on the passed in value
//    (which might be false), not to true.
//
// ****************************************************************************

void
avtSamplePointExtractor::SetRectilinearGridsAreInWorldSpace(bool val,
                 const avtViewInfo &v, double a)
{
    rectilinearGridsAreInWorldSpace = val;
    viewInfo = v;
    aspect = a;
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::RestrictToTile
//
//  Purpose:
//      Tells the extractor whether or not it should only sample within a tile.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2004
//
// ****************************************************************************

void
avtSamplePointExtractor::RestrictToTile(int wmin, int wmax, int hmin, int hmax)
{
    shouldDoTiling = true;
    width_min  = wmin;
    width_max  = wmax;
    height_min = hmin;
    height_max = hmax;
    modified = true;
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::Execute
//
//  Purpose:
//      This is the real execute method that gets the sample points out of a
//      dataset.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Sat Apr 21 13:06:27 PDT 2001 
//    Moved major portion of code to recursive Execute method that walks 
//    down the data tree.
//
//    Hank Childs, Wed Jun  6 10:31:04 PDT 2001
//    Removed domain list argument.  Blew away outdated comments.
//
//    Eric Brugger, Mon Nov  5 13:46:04 PST 2001
//    Modified to always compile the timing code.
//
//    Hank Childs, Thu Nov 15 15:39:48 PST 2001
//    Set up the extractors here (instead of constructor), since they must
//    know how many variables they are working on.
//
//    Hank Childs, Mon Nov 19 14:56:40 PST 2001
//    Gave progress while resampling.
//
// ****************************************************************************

void
avtSamplePointExtractor::Execute(void)
{
    int timingsIndex = visitTimer->StartTimer();

    SetUpExtractors();

    avtDataTree_p tree = GetInputDataTree();
    totalNodes = tree->GetNumberOfLeaves();
    currentNode = 0;
    ExecuteTree(tree);

    visitTimer->StopTimer(timingsIndex, "Sample point extraction");
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::SetUpExtractors
//
//  Purpose:
//      Sets up the extractors and tell them which volume to extract into.
//
//  Programmer: Hank Childs
//  Creation:   November 15, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jan  1 10:01:20 PST 2002
//    Tell the extractors whether they should extract from large cells.
//
//    Hank Childs, Sun Dec 14 11:07:56 PST 2003
//    Set up massVoxelExtractor.
//
//    Hank Childs, Fri Dec 10 09:59:57 PST 2004
//    Do the sampling in tiles if necessary.
//
//    Hank Childs, Sun Dec  4 19:12:42 PST 2005
//    Add support for kernel based sampling.
//
// ****************************************************************************

void
avtSamplePointExtractor::SetUpExtractors(void)
{
    avtSamplePoints_p output = GetTypedOutput();
    if (kernelBasedSampling)
        output->SetUseWeightingScheme(true);

    //
    // This will always be NULL the first time through.  For subsequent tiles
    // (provided we are doing tiling) will not have this issue.
    //
    if (output->GetVolume() == NULL)
        output->SetVolume(width, height, depth);
    else
        output->GetVolume()->ResetSamples();
    output->ResetCellList();
    avtVolume *volume = output->GetVolume();
    if (shouldDoTiling)
        volume->Restrict(width_min, width_max-1, height_min, height_max-1);

    if (hexExtractor != NULL)
    {
        delete hexExtractor;
    }
    if (massVoxelExtractor != NULL)
    {
        delete massVoxelExtractor;
    }
    if (tetExtractor != NULL)
    {
        delete tetExtractor;
    }
    if (wedgeExtractor != NULL)
    {
        delete wedgeExtractor;
    }
    if (pointExtractor != NULL)
    {
        delete pointExtractor;
    }
    if (pyramidExtractor != NULL)
    {
        delete pyramidExtractor;
    }

    //
    // Set up the extractors and tell them which cell list to use.
    //
    avtCellList *cl = output->GetCellList();
    hexExtractor = new avtHexahedronExtractor(width, height, depth, volume,cl);
    massVoxelExtractor = new avtMassVoxelExtractor(width, height, depth, volume,cl);
    tetExtractor = new avtTetrahedronExtractor(width, height, depth,volume,cl);
    wedgeExtractor = new avtWedgeExtractor(width, height, depth, volume, cl);
    pointExtractor = new avtPointExtractor(width, height, depth, volume, cl);
    pyramidExtractor = new avtPyramidExtractor(width, height, depth,volume,cl);

    hexExtractor->SendCellsMode(sendCells);
    massVoxelExtractor->SendCellsMode(sendCells);
    tetExtractor->SendCellsMode(sendCells);
    wedgeExtractor->SendCellsMode(sendCells);
    pointExtractor->SendCellsMode(sendCells);
    pyramidExtractor->SendCellsMode(sendCells);

    if (shouldDoTiling)
    {
        hexExtractor->Restrict(width_min, width_max-1, 
                               height_min, height_max-1);
        massVoxelExtractor->Restrict(width_min, width_max-1,
                                     height_min, height_max-1);
        tetExtractor->Restrict(width_min, width_max-1,
                               height_min, height_max-1);
        wedgeExtractor->Restrict(width_min, width_max-1, height_min, 
                                 height_max-1);
        pointExtractor->Restrict(width_min, width_max-1,
                                 height_min, height_max-1);
        pyramidExtractor->Restrict(width_min, width_max-1,
                                   height_min, height_max-1);
    }
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::PreExecute
//
//  Purpose:
//      Determines how many points we have if we have a point mesh.  This will
//      allow us to choose an appropriate radius.
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2006
//
// ****************************************************************************

void
avtSamplePointExtractor::PreExecute(void)
{
    avtDatasetToSamplePointsFilter::PreExecute();

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
    {
        avtDataset_p ds = GetTypedInput();
        int nzones = avtDatasetExaminer::GetNumberOfZones(ds);
        int total_nzones = UnifyMaximumValue(nzones);
        
        if (total_nzones == 0)
        {
            point_radius = 0.1;
            return;
        }

        // In image space, the total volume will be 4 (-1->+1 in X,-1->+1 in Y,
        // 0->+1 in Z).  But: we want to treat all dimensions evenly.  So
        // use 8 (doubling Z) and then correct for it later (when we use the
        // number).
        int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
        double start_vol = (dim == 3 ? 8. : 4.);
        double vol_per_point = start_vol / total_nzones;
        double exp = (dim == 3 ? 0.333333 : 0.5);
        double side_length = pow(vol_per_point, exp) / 2;
        point_radius = side_length * 1.1; // a little extra
    }
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::ExecuteTree
//
//  Purpose:
//      This is the recursive execute method that gets the sample points 
//      out of a dataset.  
//
//  Arguments:
//      dt      The data tree that should be processed.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 21, 2001. 
//
//  Modifications:
//
//    Hank Childs, Wed Jun  6 10:22:48 PDT 2001
//    Renamed ExecuteTree.
//
//    Hank Childs, Tue Jun 19 19:24:39 PDT 2001
//    Put in logic to handle bad data trees.
//
//    Hank Childs, Tue Nov 13 15:22:07 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Mon Nov 19 14:56:40 PST 2001
//    Gave progress while resampling.
//
//    Hank Childs, Mon Apr 15 15:34:43 PDT 2002
//    Give clearer error messages.
//
//    Hank Childs, Fri Jul 18 11:42:10 PDT 2003
//    Do not sample ghost zones.  This gives slightly better performance.
//    And ghost zones occasionally have the wrong value (due to problems with
//    the code that produced it).
//
//    Hank Childs, Sun Dec 14 11:07:56 PST 2003
//    Make use of massVoxelExtractor.
//
//    Hank Childs, Fri Aug 27 16:47:45 PDT 2004
//    Rename ghost data arrays.
//
//    Hank Childs, Fri Nov 19 13:57:02 PST 2004
//    If the rectilinear grids are in world space, then let the mass voxel
//    extractor know about it.
//
//    Hank Childs, Sat Jan 29 13:32:54 PST 2005
//    Added 2D extractors.
//
//    Hank Childs, Sun Jan  1 10:53:09 PST 2006
//    Moved raster based sampling into its own method.  Added support for
//    kernel based sampling.
//
// ****************************************************************************

void
avtSamplePointExtractor::ExecuteTree(avtDataTree_p dt)
{
    if (*dt == NULL)
    {
        return;
    }
    if (dt->GetNChildren() <= 0 && (!(dt->HasData())))
    {
        return;
    }

    if (dt->GetNChildren() != 0)
    {
        for (int i = 0; i < dt->GetNChildren(); i++)
        {
            if (dt->ChildIsPresent(i))
                ExecuteTree(dt->GetChild(i));
        }

        return;
    }

    //
    // Get the dataset for this leaf in the tree.
    //
    vtkDataSet *ds = dt->GetDataRepresentation().GetDataVTK();

    //
    // Iterate over all cells in the mesh and call the appropriate 
    // extractor for each cell to get the sample points.
    //
    if (kernelBasedSampling)
        KernelBasedSample(ds);
    else
        RasterBasedSample(ds);

    UpdateProgress(10*currentNode+9, 10*totalNodes);
    currentNode++;
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::KernelBasedSample
//
//  Purpose:
//      Does kernel based sampling.
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2006
//
//  Modifications:
//
//    Hank Childs, Fri Feb 24 11:22:05 PST 2006
//    Remove topological tests ... anything goes (points, lines, triangles).
//
// ****************************************************************************

void
avtSamplePointExtractor::KernelBasedSample(vtkDataSet *ds)
{
    int numCells = ds->GetNumberOfCells();
    int lastMilestone = 0;
    vtkUnsignedCharArray *ghosts = (vtkUnsignedCharArray *)
                                  ds->GetCellData()->GetArray("avtGhostZones");

    bool is2D = GetInput()->GetInfo().GetAttributes().GetSpatialDimension()==2;
    for (int j = 0 ; j < numCells ; j++)
    {
        //
        // Make sure this is a cell we should be processing.
        //
        if (ghosts != NULL && ghosts->GetValue(j) > 0)
            continue;
        vtkCell *cell = ds->GetCell(j);
        avtPoint pt;
        pt.nVars = 0;

        //
        // Get all the zonal variables.
        //
        int npts = cell->GetNumberOfPoints();
        int v;
        vtkFieldData *cd = ds->GetCellData();
        int ncd = cd->GetNumberOfArrays();
        for (v = 0 ; v < ncd ; v++)
        {
            vtkDataArray *arr = cd->GetArray(v);
            if (strstr(arr->GetName(), "vtk") != NULL)
                continue;
            if (strstr(arr->GetName(), "avt") != NULL)
                continue;
            float val = arr->GetComponent(j, 0);
            pt.val[pt.nVars] = val;
            pt.nVars++;
        }

        //
        // Turn all the nodal variables into zonal variables.
        //
        vtkFieldData *pd = ds->GetPointData();
        int npd = pd->GetNumberOfArrays();
        for (v = 0 ; v < npd ; v++)
        {
            vtkDataArray *arr = pd->GetArray(v);
            if (strstr(arr->GetName(), "vtk") != NULL)
                continue;
            if (strstr(arr->GetName(), "avt") != NULL)
                continue;
            vtkIdList *ids = cell->GetPointIds();
            double accum = 0;
            for (int i = 0 ; i < npts ; i++)
            {
                float val = arr->GetComponent(ids->GetId(i), 0);
                accum += val;
            }
            accum /= npts;
            pt.val[pt.nVars] = accum;
            pt.nVars++;
        }

        // 
        // Figure out what the bounding box is.
        //
        double bbox[6];
        if (npts > 1)
            cell->GetBounds(bbox);
        else
        {
            double pt_loc[3];
            vtkIdList *ids = cell->GetPointIds();
            vtkIdType id = ids->GetId(0);
            ds->GetPoint(id, pt_loc);
            bbox[0] = pt_loc[0]-point_radius;
            bbox[1] = pt_loc[0]+point_radius;
            bbox[2] = pt_loc[1]-point_radius;
            bbox[3] = pt_loc[1]+point_radius;
            bbox[4] = (is2D ? 0. : pt_loc[2]-point_radius/2.);
            bbox[5] = (is2D ? 0. : pt_loc[2]+point_radius/2.);
        }

        pt.bbox[0] = bbox[0];
        pt.bbox[1] = bbox[1];
        pt.bbox[2] = bbox[2];
        pt.bbox[3] = bbox[3];
        pt.bbox[4] = bbox[4];
        pt.bbox[5] = bbox[5];

        pointExtractor->Extract(pt);

        int currentMilestone = (int)(((float) j) / numCells * 10);
        if (currentMilestone > lastMilestone)
        {
            UpdateProgress(10*currentNode+currentMilestone, 10*totalNodes);
            lastMilestone = currentMilestone;
        }
    }
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::RasterBasedSample
//
//  Purpose:
//      Does raster based sampling.
//
//  Programmer: Hank Childs
//  Creation:   January 1, 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
// ****************************************************************************

void
avtSamplePointExtractor::RasterBasedSample(vtkDataSet *ds)
{
    if (modeIs3D && ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
        const double *xform = NULL;
        if (atts.GetRectilinearGridHasTransform())
            xform = atts.GetRectilinearGridTransform();
        massVoxelExtractor->SetGridsAreInWorldSpace(
           rectilinearGridsAreInWorldSpace, viewInfo, aspect, xform);
        massVoxelExtractor->Extract((vtkRectilinearGrid *) ds);
        return;
    }

    int numCells = ds->GetNumberOfCells();
    int lastMilestone = 0;
    vtkUnsignedCharArray *ghosts = (vtkUnsignedCharArray *)
                                  ds->GetCellData()->GetArray("avtGhostZones");
    for (int j = 0 ; j < numCells ; j++)
    {
        if (ghosts != NULL && ghosts->GetValue(j) > 0)
            continue;

        vtkCell *cell = ds->GetCell(j);
        if (modeIs3D && cell->GetCellDimension() != 3)
            EXCEPTION1(InvalidCellTypeException, "triangles and quads.");
        if (!modeIs3D && cell->GetCellDimension() != 2)
            EXCEPTION1(InvalidCellTypeException, "surfaces or anything outside"
                                                 " the finite element zoo.");

        switch (cell->GetCellType())
        {
          case VTK_HEXAHEDRON:
            ExtractHex((vtkHexahedron *) cell, ds, j);
            break;

          case VTK_VOXEL:
            ExtractVoxel((vtkVoxel *) cell, ds, j);
            break;
                    
          case VTK_TETRA:
            ExtractTet((vtkTetra *) cell, ds, j);
            break;

          case VTK_WEDGE:
            ExtractWedge((vtkWedge *) cell, ds, j);
            break;

          case VTK_PYRAMID:
            ExtractPyramid((vtkPyramid *) cell, ds, j);
            break;

          case VTK_TRIANGLE:
            ExtractTriangle((vtkTriangle *) cell, ds, j);
            break;

          case VTK_QUAD:
            ExtractQuad((vtkQuad *) cell, ds, j);
            break;

          case VTK_PIXEL:
            ExtractPixel((vtkPixel *) cell, ds, j);
            break;

          default:
            EXCEPTION1(InvalidCellTypeException, "surfaces or anything outside"
                                                 " the finite element zoo.");
        }
        int currentMilestone = (int)(((float) j) / numCells * 10);
        if (currentMilestone > lastMilestone)
        {
            UpdateProgress(10*currentNode+currentMilestone, 10*totalNodes);
            lastMilestone = currentMilestone;
        }
    }
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::ExtractHex
//
//  Purpose:
//      Sets up the data and call the extract method for a hexahedron.
//
//  Arguments:
//      hex      The hexahedron cell.
//      ds       The dataset the hex came from (needed to find the var).
//      hexind   The index of hex in ds.
//
//  Programmer:  Hank Childs
//  Creation:    December 5, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Nov 13 15:22:07 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Fri Dec  7 10:15:45 PST 2001
//    Added early termination for zero-opacity cells.
//
//    Hank Childs, Thu Jul 17 17:27:59 PDT 2003
//    Don't extract VTK or AVT variables.
//
// ****************************************************************************

void
avtSamplePointExtractor::ExtractHex(vtkHexahedron *hex, vtkDataSet *ds,
                                    int hexind)
{
    int  v, i;

    avtHexahedron   h;
    h.nVars = 0;

    //
    // Retrieve all of the zonal variables.
    //
    vtkFieldData *cd = ds->GetCellData();
    int ncd = cd->GetNumberOfArrays();
    for (v = 0 ; v < ncd ; v++)
    {
        vtkDataArray *arr = cd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        float val = arr->GetComponent(hexind, 0);
        for (i = 0 ; i < 8 ; i++)
        {
            h.val[i][h.nVars] = val;
        }
        h.nVars++;
    }

    //
    // Retrieve all of the nodal variables.
    //
    vtkFieldData *pd = ds->GetPointData();
    int npd = pd->GetNumberOfArrays();
    for (v = 0 ; v < npd ; v++)
    {
        vtkDataArray *arr = pd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        vtkIdList *ids = hex->GetPointIds();
        for (i = 0 ; i < 8 ; i++)
        {
            float val = arr->GetComponent(ids->GetId(i), 0);
            h.val[i][h.nVars] = val;
        }
        h.nVars++;
    }

    if (rayfoo != NULL)
    {
        if (!rayfoo->CanContributeToPicture(8, h.val))
        {
            return;
        }
    }

    //
    // Get the points for the hexahedron in our own data structure.
    //
    vtkPoints *pts = hex->GetPoints();
    for (i = 0 ; i < 8 ; i++)
    {
        double pt[3];
        pts->GetPoint(i, pt);
        h.pts[i][0] = pt[0];
        h.pts[i][1] = pt[1];
        h.pts[i][2] = pt[2];
    }

    //
    // Have the extractor extract the sample points from this hexahedron.
    //
    hexExtractor->Extract(h);
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::ExtractWedge
//
//  Purpose:
//      Sets up the data and call the extract method for a wedge.
//
//  Arguments:
//      wedge     The wedge.
//      ds        The dataset the wedge came from (needed to find the var).
//      wedgeind  The index of wedge in ds.
//
//  Programmer:   Hank Childs
//  Creation:     December 8, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Nov 13 15:22:07 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Fri Dec  7 10:15:45 PST 2001
//    Added early termination for zero-opacity cells.
//
//    Hank Childs, Thu Jul 17 17:27:59 PDT 2003
//    Don't extract VTK or AVT variables.
//
// ****************************************************************************

void
avtSamplePointExtractor::ExtractWedge(vtkWedge *wedge, vtkDataSet *ds,
                                      int wedgeind)
{
    int   i, v;

    avtWedge   w;
    w.nVars = 0;

    //
    // Retrieve all of the zonal variables.
    //
    vtkFieldData *cd = ds->GetCellData();
    int ncd = cd->GetNumberOfArrays();
    for (v = 0 ; v < ncd ; v++)
    {
        vtkDataArray *arr = cd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        float val = arr->GetComponent(wedgeind, 0);
        for (i = 0 ; i < 6 ; i++)
        {
            w.val[i][w.nVars] = val;
        }
        w.nVars++;
    }

    //
    // Retrieve all of the nodal variables.
    //
    vtkFieldData *pd = ds->GetPointData();
    int npd = pd->GetNumberOfArrays();
    for (v = 0 ; v < npd ; v++)
    {
        vtkDataArray *arr = pd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        vtkIdList *ids = wedge->GetPointIds();
        for (i = 0 ; i < 6 ; i++)
        {
            float val = arr->GetComponent(ids->GetId(i), 0);
            w.val[i][w.nVars] = val;
        }
        w.nVars++;
    }

    if (rayfoo != NULL)
    {
        if (!rayfoo->CanContributeToPicture(6, w.val))
        {
            return;
        }
    }

    //
    // Get the points for the wedge in our own data structure.
    //
    vtkPoints *pts = wedge->GetPoints();
    for (i = 0 ; i < 6 ; i++)
    {
        double pt[3];
        pts->GetPoint(i, pt);
        w.pts[i][0] = pt[0];
        w.pts[i][1] = pt[1];
        w.pts[i][2] = pt[2];
    }

    //
    // Have the extractor extract the sample points from this wedge.
    //
    wedgeExtractor->Extract(w);
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::ExtractTet
//
//  Purpose:
//      Sets up the data and call the extract method for a tetrahedron.
//
//  Arguments:
//      tet       The tetrahedron.
//      ds        The dataset the tet came from (needed to find the var).
//      tetind    The index of tet in ds.
//
//  Programmer:   Hank Childs
//  Creation:     December 8, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Nov 13 15:22:07 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Fri Dec  7 10:15:45 PST 2001
//    Added early termination for zero-opacity cells.
//
//    Hank Childs, Thu Jul 17 17:27:59 PDT 2003
//    Don't extract VTK or AVT variables.
//
// ****************************************************************************

void
avtSamplePointExtractor::ExtractTet(vtkTetra *tet, vtkDataSet *ds,
                                    int tetind)
{
    int   i, v;

    avtTetrahedron   t;
    t.nVars = 0;

    //
    // Retrieve all of the zonal variables.
    //
    vtkFieldData *cd = ds->GetCellData();
    int ncd = cd->GetNumberOfArrays();
    for (v = 0 ; v < ncd ; v++)
    {
        vtkDataArray *arr = cd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        float val = arr->GetComponent(tetind, 0);
        for (i = 0 ; i < 4 ; i++)
        {
            t.val[i][t.nVars] = val;
        }
        t.nVars++;
    }

    //
    // Retrieve all of the nodal variables.
    //
    vtkFieldData *pd = ds->GetPointData();
    int npd = pd->GetNumberOfArrays();
    for (v = 0 ; v < npd ; v++)
    {
        vtkDataArray *arr = pd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        vtkIdList *ids = tet->GetPointIds();
        for (i = 0 ; i < 4 ; i++)
        {
            float val = arr->GetComponent(ids->GetId(i), 0);
            t.val[i][t.nVars] = val;
        }
        t.nVars++;
    }

    if (rayfoo != NULL)
    {
        if (!rayfoo->CanContributeToPicture(4, t.val))
        {
            return;
        }
    }

    //
    // Get the points for the tet in our own data structure.
    //
    vtkPoints *pts = tet->GetPoints();
    for (i = 0 ; i < 4 ; i++)
    {
        double pt[3];
        pts->GetPoint(i, pt);
        t.pts[i][0] = pt[0];
        t.pts[i][1] = pt[1];
        t.pts[i][2] = pt[2];
    }

    //
    // Have the extractor extract the sample points from this tetrahedron.
    //
    tetExtractor->Extract(t);
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::ExtractPyramid
//
//  Purpose:
//      Sets up the data and call the extract method for a pyramid.
//
//  Arguments:
//      pyr       The pyramid.
//      ds        The dataset the pyramid came from (needed to find the var).
//      pyrind    The index of pyramid in ds.
//
//  Programmer:   Hank Childs
//  Creation:     December 8, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Nov 13 15:22:07 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Fri Dec  7 10:15:45 PST 2001
//    Added early termination for zero-opacity cells.
//
//    Hank Childs, Thu Jul 17 17:27:59 PDT 2003
//    Don't extract VTK or AVT variables.
//
// ****************************************************************************

void
avtSamplePointExtractor::ExtractPyramid(vtkPyramid *pyr, vtkDataSet *ds,
                                       int pyrind)
{
    int   i, v;

    avtPyramid   p;
    p.nVars = 0;

    //
    // Retrieve all of the zonal variables.
    //
    vtkFieldData *cd = ds->GetCellData();
    int ncd = cd->GetNumberOfArrays();
    for (v = 0 ; v < ncd ; v++)
    {
        vtkDataArray *arr = cd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        float val = arr->GetComponent(pyrind, 0);
        for (i = 0 ; i < 5 ; i++)
        {
            p.val[i][p.nVars] = val;
        }
        p.nVars++;
    }

    //
    // Retrieve all of the nodal variables.
    //
    vtkFieldData *pd = ds->GetPointData();
    int npd = pd->GetNumberOfArrays();
    for (v = 0 ; v < npd ; v++)
    {
        vtkDataArray *arr = pd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        vtkIdList *ids = pyr->GetPointIds();
        for (i = 0 ; i < 5 ; i++)
        {
            float val = arr->GetComponent(ids->GetId(i), 0);
            p.val[i][p.nVars] = val;
        }
        p.nVars++;
    }

    if (rayfoo != NULL)
    {
        if (!rayfoo->CanContributeToPicture(5, p.val))
        {
            return;
        }
    }

    //
    // Get the points for the pyramid in our own data structure.
    //
    vtkPoints *pts = pyr->GetPoints();
    for (i = 0 ; i < 5 ; i++)
    {
        double pt[3];
        pts->GetPoint(i, pt);
        p.pts[i][0] = pt[0];
        p.pts[i][1] = pt[1];
        p.pts[i][2] = pt[2];
    }

    //
    // Have the extractor extract the sample points from this pyramid.
    //
    pyramidExtractor->Extract(p);
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::ExtractVoxel
//
//  Purpose:
//      Sets up the data and call the extract method for a voxel
//
//  Arguments:
//      voxel    The voxel cell.
//      ds       The dataset the voxel came from (needed to find the var).
//      voxind   The index of voxel in ds.
//
//  Programmer:  Hank Childs
//  Creation:    December 7, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Nov 13 15:22:07 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Fri Dec  7 10:15:45 PST 2001
//    Added early termination for zero-opacity cells.
//
//    Hank Childs, Thu Jul 17 17:27:59 PDT 2003
//    Don't extract VTK or AVT variables.
//
// ****************************************************************************

void
avtSamplePointExtractor::ExtractVoxel(vtkVoxel *voxel, vtkDataSet *ds,
                                      int voxind)
{
    int   i, v;

    avtHexahedron   h;
    h.nVars = 0;

    //
    // Retrieve all of the zonal variables.
    //
    vtkFieldData *cd = ds->GetCellData();
    int ncd = cd->GetNumberOfArrays();
    for (v = 0 ; v < ncd ; v++)
    {
        vtkDataArray *arr = cd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        float val = arr->GetComponent(voxind, 0);
        for (i = 0 ; i < 8 ; i++)
        {
            h.val[i][h.nVars] = val;
        }
        h.nVars++;
    }

    //
    // vth takes voxels to hexahedrons.
    //
    int  vth[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };

    //
    // Retrieve all of the nodal variables.
    //
    vtkFieldData *pd = ds->GetPointData();
    int npd = pd->GetNumberOfArrays();
    for (v = 0 ; v < npd ; v++)
    {
        vtkDataArray *arr = pd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        vtkIdList *ids = voxel->GetPointIds();
        for (i = 0 ; i < 8 ; i++)
        {
            float val = arr->GetComponent(ids->GetId(i), 0);
            h.val[vth[i]][h.nVars] = val;
        }
        h.nVars++;
    }

    if (rayfoo != NULL)
    {
        if (!rayfoo->CanContributeToPicture(8, h.val))
        {
            return;
        }
    }

    //
    // Get the points for the hexahedron in our own data structure.
    //
    vtkPoints *pts = voxel->GetPoints();
    for (i = 0 ; i < 8 ; i++)
    {
        double pt[3];
        pts->GetPoint(i, pt);
        h.pts[vth[i]][0] = pt[0];
        h.pts[vth[i]][1] = pt[1];
        h.pts[vth[i]][2] = pt[2];
    }

    //
    // Have the extractor extract the sample points from this hexahedron.
    //
    hexExtractor->Extract(h);
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::ExtractTriangle
//
//  Purpose:
//      Sets up the data and call the extract method for a triangle
//
//  Arguments:
//      tri      The triangle cell.
//      ds       The dataset the triangle came from (needed to find the var).
//      triind   The index of the triangle in ds.
//
//  Programmer:  Hank Childs
//  Creation:    January 29, 2005
//
// ****************************************************************************

void
avtSamplePointExtractor::ExtractTriangle(vtkTriangle *tri, vtkDataSet *ds,
                                         int triind)
{
    int   i, v;

    avtWedge w;
    w.nVars = 0;

    //
    // Retrieve all of the zonal variables.
    //
    vtkFieldData *cd = ds->GetCellData();
    int ncd = cd->GetNumberOfArrays();
    for (v = 0 ; v < ncd ; v++)
    {
        vtkDataArray *arr = cd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        float val = arr->GetComponent(triind, 0);
        for (i = 0 ; i < 6 ; i++)
        {
            w.val[i][w.nVars] = val;
        }
        w.nVars++;
    }

    //
    // Retrieve all of the nodal variables.
    //
    vtkFieldData *pd = ds->GetPointData();
    int npd = pd->GetNumberOfArrays();
    for (v = 0 ; v < npd ; v++)
    {
        vtkDataArray *arr = pd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        vtkIdList *ids = tri->GetPointIds();
        float val0 = arr->GetComponent(ids->GetId(0), 0);
        float val1 = arr->GetComponent(ids->GetId(1), 0);
        float val2 = arr->GetComponent(ids->GetId(2), 0);
        w.val[0][w.nVars] = val0;
        w.val[1][w.nVars] = val1;
        w.val[2][w.nVars] = val2;
        w.val[3][w.nVars] = val0;
        w.val[4][w.nVars] = val1;
        w.val[5][w.nVars] = val2;
        w.nVars++;
    }

    if (rayfoo != NULL)
    {
        if (!rayfoo->CanContributeToPicture(6, w.val))
        {
            return;
        }
    }

    //
    // Get the points for the triangle in our own data structure.
    //
    vtkPoints *pts = tri->GetPoints();
    for (i = 0 ; i < 3 ; i++)
    {
        double pt[3];
        pts->GetPoint(i, pt);
        w.pts[i][0] = pt[0];
        w.pts[i][1] = pt[1];
        w.pts[i][2] = pt[2]-0.1;  // Make the cell be 3D
        w.pts[3+i][0] = pt[0];
        w.pts[3+i][1] = pt[1];
        w.pts[3+i][2] = pt[2]+0.1; // Make the cell be 3D
    }

    //
    // Have the extractor extract the sample points from this triangle.
    //
    wedgeExtractor->Extract(w);
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::ExtractQuad
//
//  Purpose:
//      Sets up the data and call the extract method for a quad
//
//  Arguments:
//      quad     The quad cell.
//      ds       The dataset the triangle came from (needed to find the var).
//      quadind  The index of the quad in ds.
//
//  Programmer:  Hank Childs
//  Creation:    January 29, 2005
//
// ****************************************************************************

void
avtSamplePointExtractor::ExtractQuad(vtkQuad *quad, vtkDataSet *ds,
                                     int quadind)
{
    int   i, v;

    avtHexahedron h;
    h.nVars = 0;

    //
    // Retrieve all of the zonal variables.
    //
    vtkFieldData *cd = ds->GetCellData();
    int ncd = cd->GetNumberOfArrays();
    for (v = 0 ; v < ncd ; v++)
    {
        vtkDataArray *arr = cd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        float val = arr->GetComponent(quadind, 0);
        for (i = 0 ; i < 8 ; i++)
        {
            h.val[i][h.nVars] = val;
        }
        h.nVars++;
    }

    //
    // Retrieve all of the nodal variables.
    //
    vtkFieldData *pd = ds->GetPointData();
    int npd = pd->GetNumberOfArrays();
    for (v = 0 ; v < npd ; v++)
    {
        vtkDataArray *arr = pd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        vtkIdList *ids = quad->GetPointIds();
        float val0 = arr->GetComponent(ids->GetId(0), 0);
        float val1 = arr->GetComponent(ids->GetId(1), 0);
        float val2 = arr->GetComponent(ids->GetId(2), 0);
        float val3 = arr->GetComponent(ids->GetId(3), 0);
        h.val[0][h.nVars] = val0;
        h.val[1][h.nVars] = val1;
        h.val[2][h.nVars] = val2;
        h.val[3][h.nVars] = val3;
        h.val[4][h.nVars] = val0;
        h.val[5][h.nVars] = val1;
        h.val[6][h.nVars] = val2;
        h.val[7][h.nVars] = val3;
        h.nVars++;
    }

    if (rayfoo != NULL)
    {
        if (!rayfoo->CanContributeToPicture(8, h.val))
        {
            return;
        }
    }

    //
    // Get the points for the triangle in our own data structure.
    //
    vtkPoints *pts = quad->GetPoints();
    for (i = 0 ; i < 4 ; i++)
    {
        double pt[3];
        pts->GetPoint(i, pt);
        h.pts[i][0] = pt[0];
        h.pts[i][1] = pt[1];
        h.pts[i][2] = pt[2]-0.1;  // Make the cell be 3D
        h.pts[4+i][0] = pt[0];
        h.pts[4+i][1] = pt[1];
        h.pts[4+i][2] = pt[2]+0.1; // Make the cell be 3D
    }

    //
    // Have the extractor extract the sample points from this quad.
    //
    hexExtractor->Extract(h);
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::ExtractPixel
//
//  Purpose:
//      Sets up the data and call the extract method for a pixel
//
//  Arguments:
//      pixel    The pixel cell.
//      ds       The dataset the triangle came from (needed to find the var).
//      pixind   The index of the pixel in ds.
//
//  Programmer:  Hank Childs
//  Creation:    January 29, 2005
//
// ****************************************************************************

void
avtSamplePointExtractor::ExtractPixel(vtkPixel *pixel, vtkDataSet *ds,
                                     int pixind)
{
    int   i, v;

    avtHexahedron h;
    h.nVars = 0;

    //
    // Retrieve all of the zonal variables.
    //
    vtkFieldData *cd = ds->GetCellData();
    int ncd = cd->GetNumberOfArrays();
    for (v = 0 ; v < ncd ; v++)
    {
        vtkDataArray *arr = cd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        float val = arr->GetComponent(pixind, 0);
        for (i = 0 ; i < 8 ; i++)
        {
            h.val[i][h.nVars] = val;
        }
        h.nVars++;
    }

    //
    // Retrieve all of the nodal variables.
    //
    vtkFieldData *pd = ds->GetPointData();
    int npd = pd->GetNumberOfArrays();
    for (v = 0 ; v < npd ; v++)
    {
        vtkDataArray *arr = pd->GetArray(v);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        vtkIdList *ids = pixel->GetPointIds();
        float val0 = arr->GetComponent(ids->GetId(0), 0);
        float val1 = arr->GetComponent(ids->GetId(1), 0);
        float val2 = arr->GetComponent(ids->GetId(2), 0);
        float val3 = arr->GetComponent(ids->GetId(3), 0);
        h.val[0][h.nVars] = val0;
        h.val[1][h.nVars] = val1;
        h.val[2][h.nVars] = val3;  // Swap for pixel
        h.val[3][h.nVars] = val2;  // Swap for pixel
        h.val[4][h.nVars] = val0;
        h.val[5][h.nVars] = val1;
        h.val[6][h.nVars] = val3;  // Swap for pixel
        h.val[7][h.nVars] = val2;  // Swap for pixel
        h.nVars++;
    }

    if (rayfoo != NULL)
    {
        if (!rayfoo->CanContributeToPicture(8, h.val))
        {
            return;
        }
    }

    //
    // Get the points for the triangle in our own data structure.
    //
    vtkPoints *pts = pixel->GetPoints();
    int pix2quad[4] = { 0, 1, 3, 2 };
    for (i = 0 ; i < 4 ; i++)
    {
        double pt[3];
        pts->GetPoint(pix2quad[i], pt);
        h.pts[i][0] = pt[0];
        h.pts[i][1] = pt[1];
        h.pts[i][2] = pt[2]-0.1;  // Make the cell be 3D
        h.pts[4+i][0] = pt[0];
        h.pts[4+i][1] = pt[1];
        h.pts[4+i][2] = pt[2]+0.1; // Make the cell be 3D
    }

    //
    // Have the extractor extract the sample points from this pixel.
    //
    hexExtractor->Extract(h);
}


// ****************************************************************************
//  Method: avtSamplePointExtractor::SendCellsMode
//
//  Purpose:
//      Tell the individual cell extractors that it is okay to send the cells
//      instead of sample points when doing parallel volume rendering.
//
//  Arguments:
//      mode    true if the cell extractors should serialize the cells and send
//              those.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jan  1 10:01:20 PST 2002
//    Reflect that extractors may not be initialized at this point.
//
//    Hank Childs, Tue Jan 24 16:42:40 PST 2006
//    Add support for kernel based sampling.
//
// ****************************************************************************

void
avtSamplePointExtractor::SendCellsMode(bool mode)
{
    sendCells = mode;

    if (hexExtractor != NULL)
    {
        hexExtractor->SendCellsMode(sendCells);
    }
    if (pointExtractor != NULL)
    {
        pointExtractor->SendCellsMode(sendCells);
    }
    if (pyramidExtractor != NULL)
    {
        pyramidExtractor->SendCellsMode(sendCells);
    }
    if (tetExtractor != NULL)
    {
        tetExtractor->SendCellsMode(sendCells);
    }
    if (wedgeExtractor != NULL)
    {
        wedgeExtractor->SendCellsMode(sendCells);
    }
}


// ****************************************************************************
//  Method:  avtSamplePointExtractor::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************

bool
avtSamplePointExtractor::FilterUnderstandsTransformedRectMesh()
{
    // Raster-based sampling has been extended to handle these
    // meshes.  Kernel-based has not.
    if (kernelBasedSampling)
        return false;
    else
        return true;
}
