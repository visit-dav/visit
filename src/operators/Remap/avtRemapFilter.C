// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtRemapFilter.C
// ************************************************************************* //

#include <avtRemapFilter.h>

#include <vector>

#include <vtkDataArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPlane.h>
#include <vtkCell.h>
#include <vtkCellData.h>

#include <verdict.h>

#include <avtExtents.h>
#include <avtDataAttributes.h>
#include <avtDataTree.h>
#include <avtParallel.h>
#include <vtkVisItClipper.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtRemapFilter constructor
//
//  Programmer: rusu1
//  Creation:   Wed Apr  3 13:52:34 PDT 2019
//
// ****************************************************************************

avtRemapFilter::avtRemapFilter()
: rGridBounds{0., 0., 0., 0., 0., 0.},
  rCellVolume(0.),
  is3D(false)
{
    debug4 << "avtRemapFilter::avtRemapFilter" << std::endl;
    rg = vtkRectilinearGrid::New();
    vars = vtkDoubleArray::New();
}


// ****************************************************************************
//  Method: avtRemapFilter destructor
//
//  Programmer: rusu1
//  Creation:   Wed Apr  3 13:52:34 PDT 2019
//
// ****************************************************************************

avtRemapFilter::~avtRemapFilter()
{
    debug4 << "avtRemapFilter::~avtRemapFilter" << std::endl;
    rg->Delete();
    vars->Delete();
}


// ****************************************************************************
//  Method: avtRemapFilter::Create
//
//  Programmer: rusu1
//  Creation:   Wed Apr  3 13:52:34 PDT 2019
//
// ****************************************************************************

avtFilter *
avtRemapFilter::Create()
{
    debug4 << "avtRemapFilter::Create" << endl;
    return new avtRemapFilter();
}


// ****************************************************************************
//  Method: avtRemapFilter::SetAtts
//
//  Purpose:
//      Sets the state of the filter based on the attribute object.
//
//  Arguments:
//      a        The attributes to use.
//
//  Programmer: rusu1
//  Creation:   Wed Apr  3 13:52:34 PDT 2019
//
// ****************************************************************************

void
avtRemapFilter::SetAtts(const AttributeGroup *a)
{
    debug4 << "avtRemapFilter::SetAtts" << std::endl;
    atts = *(const RemapAttributes*)a;
}


// ****************************************************************************
//  Method: avtRemapFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtRemapFilter where the given
//      parameters would result in an equivalent avtRemapFilter.
//
//  Programmer: rusu1
//  Creation:   Wed Apr  3 13:52:34 PDT 2019
//
// ****************************************************************************

bool
avtRemapFilter::Equivalent(const AttributeGroup *a)
{
    debug4 << "avtRemapFilter::Equivalent(" << std::endl;
    return (atts == *(RemapAttributes*)a);
}


// ****************************************************************************
//  Method: avtRemapFilter::Execute
//
//  Purpose:
//      Execute the Filter.
//
//  Programmer: rusu1
//  Creation:   Wed Apr  3 13:52:34 PDT 2019
//
//  Modifications:
//      Eddie Rusu, Tue Jul 14 10:04:57 PDT 2020
//      Execute uses GetAllLeaves() instead of recursive traverse domain.
//
//      Eddie Rusu, Thu Jul 16 11:09:52 PDT 2020
//      Added parallel support.
//
// ****************************************************************************

void
avtRemapFilter::Execute(void)
{
    debug1 << "avtRemapFilter::Execute" << std::endl;    

    // --------------------------------- //
    // --- Generate Rectilinear Grid --- //
    // --------------------------------- //
    GetBounds();
    int width = atts.GetCellsX();
    int height = atts.GetCellsY();
    int depth;
    
    // Setup whatever variables I can assuming the grid is 2D. Then, check if it
    // is actually 3D. If so, modify parameters and build rg. Otherwise, build
    // rg with 2D variable values.
    int nCellsOut = width*height;
    rCellVolume = (rGridBounds[1] - rGridBounds[0]) *
                  (rGridBounds[3] - rGridBounds[2]) /
                  (nCellsOut);    
    if (is3D) 
    {
        debug5 << "Generating 3D grid" << std::endl;
        depth = atts.GetCellsZ();
        nCellsOut *= depth;
        rCellVolume *= (rGridBounds[5] - rGridBounds[4]) / depth;
        CreateGrid(width, height, depth, 0, width, 0, height, 0, depth);
    }
    else
    {
        debug5 << "Generating 2D grid" << std::endl;
        CreateGrid(width, height, 0, width, 0, height);
    }
    
    // If there are no variables, then just create the mesh and exit
    int nVariables = GetInput()->GetInfo().GetAttributes().GetNumberOfVariables();
    if (nVariables <= 0)
    {
        debug5 << "There are no variables. Outputting empty mesh." << std::endl;
        SetOutputDataTree(new avtDataTree(rg, 0));
        return;
    }
    
    // Add variables to the rectilinear grid   
    vars->SetNumberOfComponents(1);
    vars->SetNumberOfTuples(nCellsOut);
    for (int i = 0; i < vars->GetNumberOfTuples(); ++i)
    {
        vars->SetComponent(i, 0, 0); // Initialize vars to 0
    }
    vars->SetName(GetInput()->GetInfo().GetAttributes().GetVariableName().c_str());
    rg->GetCellData()->AddArray(vars);
    rg->GetCellData()->SetScalars(vars);
    debug5 << "Variable " << vars->GetName() << " added to grid." << std::endl;
    

    // ----------------------------------------------------------- //
    // --- Setup the clipping planes for the recitilinear grid --- //
    // ----------------------------------------------------------- //
    
    // Get the planes for first dimension
    debug5 << "Creating clipping functions" << std::endl;
    MakeClippingFunction(0, 0); // Get the leftmost plane
    for (int rCell = 0; rCell < width; ++rCell)
    {
        MakeClippingFunction(rCell, 1); // Get all the planes on the "right"
    }

    // Get the planes for the second dimension
    MakeClippingFunction(0, 2); // Get the topmost plane
    for (int rCell = 0; rCell < width*height; rCell+=width)
    {
        MakeClippingFunction(rCell, 3); // Get all the planes on the "bottom"
    }

    if (is3D)
    {
        // Get the planes for the third dimension
        MakeClippingFunction(0, 4); // Get the frontmost plane
        for (int rCell = 0; rCell < width*height*depth; rCell+=width*height)
        {
            MakeClippingFunction(rCell, 5); // Get all the planes on the "back"
        }
    }


    // ------------------------------------ //
    // --- Unravel the domains and clip --- //
    // ------------------------------------ //

    avtDataTree_p inTree = GetInputDataTree();
    int totalNodes;
    vtkDataSet **dataSets = inTree->GetAllLeaves(totalNodes);
    for (int i = 0; i < totalNodes; ++i)
    {
        ClipDomain(dataSets[i]);
    }


    // ---------------------------------------------- //
    // --- Gather information in parallel setting --- //
    // ---------------------------------------------- //

#ifdef PARALLEL
    int size = vars->GetNumberOfTuples();
    double *varsDouble = (double*) vars->GetVoidPointer(0);
    double *newBuff = new double[size];
    SumDoubleArray(varsDouble, newBuff, size);
    if (PAR_Rank() == 0)
    {
        for (int i = 0; i < size; ++i)
        {
            varsDouble[i] = newBuff[i];
        }
    }
    delete [] newBuff;
#endif

    if (PAR_Rank() == 0)
    {
        SetOutputDataTree(new avtDataTree(rg, 0));
    }
    else
    {
        SetOutputDataTree(new avtDataTree());
    }

    CleanClippingFunctions();
    debug5 << "DONE Remapping" << std::endl;
    return;
}


// ****************************************************************************
//  Method: avtRemapFilter::ClipDomain
//
//  Purpose:
//      Clip the input domain against the output rectilinear grid.
//
//  Arguments:
//      in_ds_tmp     vtkDataSet*, as called in Execute
//
//  Programmer: Eddie Rusu
//  Creation:   Wed Apr  3 13:52:34 PDT 2019
//
//  Modifications:
//      Eddie Rusu, Tue Jul 14 10:04:57 PDT 2020
//      Clip domain receives the vtkDataSet directly instead of the leaf.
//
//      Alister Maguire, Fri Nov 13 14:07:54 PST 2020
//      Updated the clipper to set the correct cell clip strategy.
//
// ****************************************************************************

void
avtRemapFilter::ClipDomain(vtkDataSet *in_ds_tmp)
{
    debug3 << "avtRemapFilter::ClipDomain" << std::endl;

    // Must create a new copy like this because we will modify the dataset
    // by adding new variables to it. If we don't copy like this, then we will
    // crash the engine.
    vtkDataSet* in_ds = in_ds_tmp->NewInstance();
    in_ds->ShallowCopy(in_ds_tmp);
    
    // If the dataset does not exist, then return.
    // If the variable does not exist, then return.
    // If there are no points, then return.
    // If there are no cells, then return.
    if (in_ds == NULL || in_ds->GetCellData()->GetArray(vars->GetName()) == NULL ||
        in_ds->GetNumberOfPoints() == 0 || in_ds->GetNumberOfCells() == 0)
    {
        debug4 << "This domain is invalid for remapping." << std::endl;
        return;
    }
    
    
    // ------------------------------------------------------- //
    // --- Calculate volumes of each cell in Original Grid --- //
    // ------------------------------------------------------- //
    
    // Add the original volume to the in_ds AFTER the variables have already
    // been added to rg so that we don't add an artificial variable.
    vtkDoubleArray* avtRemapOriginalVolume = CalculateCellVolumes(in_ds,
            "avtRemapOriginalVolume");
    in_ds->GetCellData()->AddArray(avtRemapOriginalVolume);
    debug5 << "Added cell volumes to in_ds" << std::endl;
    
    
    // -------------------------------------------------------- //
    // --- Clip the input grid against the rectilinear grid --- //
    // -------------------------------------------------------- //
    
    // + ----- +
    // |   3   |
    // |0     1|
    // |   2   |
    // + ----- +

    // --- Instantiate the clippers --- //
    vtkVisItClipper* last = NULL;
    vtkVisItClipper* clipperLeft = vtkVisItClipper::New();
    vtkVisItClipper* clipperRight = vtkVisItClipper::New();
    vtkVisItClipper* clipperTop = vtkVisItClipper::New();
    vtkVisItClipper* clipperBottom = vtkVisItClipper::New();
    vtkVisItClipper* clipperFront = vtkVisItClipper::New();
    vtkVisItClipper* clipperBack = vtkVisItClipper::New();

    
    // --- Loop over each cell --- //
    debug5 << "Beginning the clipping loop" << std::endl;
    for (int k = 0; k < (is3D ? atts.GetCellsZ() : 1); ++k) // Loop over z-dim
    {
        for (int j = 0; j < atts.GetCellsY(); ++j) // Loop over y-dim
        {
            for (int i = 0; i < atts.GetCellsX(); ++i) // Loop over i-dim
            {
                // Get the index for rg
                int rCell = i +
                        atts.GetCellsX() * j +
                        atts.GetCellsX() * atts.GetCellsY() * k;

                // --- Clip the cell --- //

                // Left plane
                clipperLeft->SetInputData(in_ds);
                clipperLeft->SetClipFunction(funcsArrayX[i]);
                clipperLeft->SetInsideOut(false);
                clipperLeft->SetCellClipStrategyToRemovePartial();
                last = clipperLeft;

                // Right plane
                clipperRight->SetInputData(in_ds);
                clipperRight->SetClipFunction(funcsArrayX[i+1]);
                clipperRight->SetInsideOut(true);
                clipperRight->SetCellClipStrategyToRemovePartial();
                clipperRight->SetInputConnection(last->GetOutputPort());
                last = clipperRight;

                // Top plane
                clipperTop->SetInputData(in_ds);
                clipperTop->SetClipFunction(funcsArrayY[j]);
                clipperTop->SetInsideOut(false);
                clipperTop->SetCellClipStrategyToRemovePartial();
                clipperTop->SetInputConnection(last->GetOutputPort());
                last = clipperTop;

                // Bottom plane
                clipperBottom->SetInputData(in_ds);
                clipperBottom->SetClipFunction(funcsArrayY[j+1]);
                clipperBottom->SetInsideOut(true);
                clipperBottom->SetCellClipStrategyToRemovePartial();
                clipperBottom->SetInputConnection(last->GetOutputPort());
                last = clipperBottom;

                if (is3D)
                {
                    // Front plane
                    clipperFront->SetInputData(in_ds);
                    clipperFront->SetClipFunction(funcsArrayZ[k]);
                    clipperFront->SetInsideOut(false);
                    clipperFront->SetCellClipStrategyToRemovePartial();
                    clipperFront->SetInputConnection(last->GetOutputPort());
                    last = clipperFront;

                    // Back plane
                    clipperBack->SetInputData(in_ds);
                    clipperBack->SetClipFunction(funcsArrayZ[k+1]);
                    clipperBack->SetInsideOut(true);
                    clipperBack->SetCellClipStrategyToRemovePartial();
                    clipperBack->SetInputConnection(last->GetOutputPort());
                    last = clipperBack;
                }

                last->Update();
                vtkUnstructuredGrid* ug = last->GetOutput();
                debug5 << "Cell " << rCell << " has been clipped." << std::endl;
                // Done clipping
        
                // Collection of clipped cells from the original grid that now take the shape
                // of the rgrid cell, could be something like this:
                // + -------------- +
                // |   |      /     |
                // | 1 | 3   /  5   |
                // |   ^    /------ |
                // |  / \  /    |   |
                // | /   \/  4  | 6 |
                // |/  2  \     |   |
                // + -------------- +
    
                // --- Calculate volume of subcells --- //
                // Now that we have the unstrucutred grid from the clipping, we can loop
                // over the subcells in that grid and calculate the volumes. We also need
                // the ratio between old and new volumes to weigh the variables (like mass,
                // density, pressure, etc.).

                vtkDoubleArray* subCellVolumes = CalculateCellVolumes(ug, "subCellVolume");
                debug5 << "Calculated volumes of subCells" << std::endl;

                // Get the volume of the original cells for this ugrid
                vtkDoubleArray* originalCellVolumes = vtkDoubleArray::SafeDownCast(
                    ug->GetCellData()->GetArray("avtRemapOriginalVolume"));
                // Done calculating subcell volumes


                // --- Calculate variable updates --- //
                // Two types of variables: intrinsic (like density) and extrinsic (like
                // mass). To update the variables:
                //        (1) Intrinsic values must be made extrinsic within the volume of
                //            the sub cell, then totaled among the set of sub cells, then
                //            made intrinsic again within the volume of the new cell.
                //        (2) Extrinsic values must be made intrisic within the volume of
                //            original cell, then made extrinsic within the volume of the
                //            sub cell, then totaled among the set of sub cells.
                
                double value = 0.0;
                vtkDataArray* myVariable = ug->GetCellData()->GetArray(vars->GetName());
                if (myVariable == NULL)
                { // in_ds has the variable, but ug does not
                    debug4 << "Clipped cell-grid does not have the variable" << std::endl;
                    continue; // Go to the next rgrid cell
                }
                vtkDataArray* ghostVariable = ug->GetCellData()->GetArray("avtGhostZones");
                if (atts.GetVariableType() == RemapAttributes::extrinsic) // like mass
                {
                    for (int tuple = 0;
                         tuple < myVariable->GetNumberOfTuples(); tuple++)
                    {
                        if (ghostVariable != NULL &&
                            ghostVariable->GetComponent(tuple, 0) == 1)
                        { // Ignore ghost cells
                            continue;
                        }
                        else
                        {
                            value += myVariable->GetComponent(tuple, 0) / 
                                originalCellVolumes->GetComponent(tuple, 0) *
                                subCellVolumes->GetComponent(tuple, 0);
                        }
                    }
                    vars->SetComponent(rCell, 0, value + vars->GetComponent(rCell, 0));
                }
                else // intrinsic (like density) is the default
                {
                    for (int tuple = 0;
                         tuple < myVariable->GetNumberOfTuples(); tuple++)
                    {
                        if (ghostVariable != NULL &&
                            ghostVariable->GetComponent(tuple, 0) == 1)
                        { // Ignore ghost cells
                            continue;
                        }
                        else
                        {
                            value += myVariable->GetComponent(tuple, 0) *
                                subCellVolumes->GetComponent(tuple, 0);
                        }
                    }
                    value /= rCellVolume;
                    vars->SetComponent(rCell, 0, value + vars->GetComponent(rCell, 0));
                }
                // Done updating variable

                subCellVolumes->Delete();
                clipperLeft->RemoveAllInputs();
                clipperRight->RemoveAllInputs();
                clipperTop->RemoveAllInputs();
                clipperBottom->RemoveAllInputs();
                clipperFront->RemoveAllInputs();
                clipperBack->RemoveAllInputs();
            }
        }
    } // Finished looping over cells
    clipperLeft->Delete();
    clipperRight->Delete();
    clipperTop->Delete();
    clipperBottom->Delete();
    clipperFront->Delete();
    clipperBack->Delete();
    in_ds->Delete();
} // End ClipDomain


// ****************************************************************************
//  Method: avtRemapFilter::MakeClippingFunction
//
//  Purpose:
//      Generate vtkImplicitBooleans to store in the x, y, z array of functions
//      that will be used for clipping.
//
//  Arguments:
//      rCell      The id of the current cell.
//      side       The "side" of the cell as an integer.
//
//  Programmer: rusu1
//  Creation:   Wed Apr  3 13:52:34 PDT 2019
//
// ****************************************************************************

void
avtRemapFilter::MakeClippingFunction(int rCell, int side)
{
    debug4 << "avtRemapFilter::MakeClippingFunction" << std::endl;
    double cellBounds[6] = {0., 0., 0., 0., 0., 0.};
    double origin[3] = {0., 0., 0.};
    double normal[3] = {0., 0., 0.};
    rg->GetCell(rCell)->GetBounds(cellBounds);
    origin[side/2] = cellBounds[side];
    normal[side/2] = 1.;

    vtkPlane* plane = vtkPlane::New();
    plane->SetOrigin(origin);
    plane->SetNormal(normal);
    vtkImplicitBoolean* funcs = vtkImplicitBoolean::New();
    funcs->AddFunction(plane);
    plane->Delete();

    if (side == 0 || side == 1)
    {
        funcsArrayX.push_back(funcs);
    }
    else if (side == 2 || side == 3)
    {
        funcsArrayY.push_back(funcs);
    }
    else //if (side == 4 || side == 5)
    { 
        funcsArrayZ.push_back(funcs);
    }
}


// ****************************************************************************
//  Method: avtRemapFilter::CleanClippingFunctions
//
//  Purpose:
//      Clean out all the vtkImplicitBooleans from the funcs arrays.
//
//  Programmer: rusu1
//  Creation:   Mon Apr 15 10:56:53 PDT 2019
//
// ****************************************************************************

void
avtRemapFilter::CleanClippingFunctions()
{
    debug4 << "avtRemapFilter::CleanClippingFunctions" << std::endl;

    for (std::vector<vtkImplicitBoolean*>::iterator iter = funcsArrayX.begin();
         iter != funcsArrayX.end(); ++iter)
    {
        (*iter)->Delete();
    }
    for (std::vector<vtkImplicitBoolean*>::iterator iter = funcsArrayY.begin();
         iter != funcsArrayY.end(); ++iter)
    {
        (*iter)->Delete();
    }
    for (std::vector<vtkImplicitBoolean*>::iterator iter = funcsArrayZ.begin();
         iter != funcsArrayZ.end(); ++iter)
    {
        (*iter)->Delete();
    }
}


// ****************************************************************************
//  Method: avtRemapFilter::CalculateCellVolumes
//
//  Purpose:
//      Calculates the volumes of the cells from a vtkDataSet. Stores the
//      volumes in a vtkDoubleArray.
//
//  Arguments:
//      in_ds      vtkDataSet* that has all the cells.
//      name       name of the vtkDataSet.
//
//  Returns: a vtkDoubleArray* that stores the volumes of each cell.
//
//  Programmer: rusu1
//  Creation:   Wed Apr  3 13:52:34 PDT 2019
//
// ****************************************************************************

inline void
Swap1(double &a, double &b)
{
    double tmp = a;
    a = b;
    b = tmp;
}

inline void
Swap3(double c[][3], int a, int b)
{
    Swap1(c[a][0], c[b][0]);
    Swap1(c[a][1], c[b][1]);
    Swap1(c[a][2], c[b][2]);
}

inline void
Copy3(double coords[][3], double a[], int i)
{
    a[0] = coords[i][0];
    a[1] = coords[i][1];
    a[2] = coords[i][2];
}

vtkDoubleArray*
avtRemapFilter::CalculateCellVolumes(vtkDataSet* in_ds, const char* name)
{
    debug4 << "avtRemapFilter::CalculateCellVolumes" << std::endl;
    vtkDoubleArray* volumeArray = vtkDoubleArray::New(); // return element
    
    // Set up necessary variables
    const int MAXPOINTS = 100; // Set maximum number of points per cell to 100
    double coordinates[MAXPOINTS][3]; // array of points for each cell in 3D
    
    // Get information from the mesh
    int nCells = in_ds->GetNumberOfCells();
    
    // Setup the volumeArray
    volumeArray->SetName(name);
    volumeArray->SetNumberOfComponents(1);
    volumeArray->SetNumberOfTuples(nCells);
    
    for (int n = 0; n < nCells; n++)
    {
        double volume = 0.0;
        vtkCell* cell = in_ds->GetCell(n); // Get the cell
        vtkDataArray* pointData = cell->GetPoints()->GetData(); // Get the points
        for (vtkIdType j = 0; j < cell->GetNumberOfPoints(); j++)
        {
            coordinates[j][2] = 0; // Set to 0 in case 2D
            pointData->GetTuple(j, coordinates[j]); // Set the j-th entry in coordiantes to the tuple from pointData at j
        }
        
        int subdiv[3][4] = { {0,5,4,3}, {0,2,1,4}, {0,4,5,2} };
        // ^ Need this to be before switch otherwise ill-formed program.
        switch(cell->GetCellType())
        {
            case VTK_TRIANGLE:
                volume = v_tri_area(3, coordinates);
                break;
            case VTK_QUAD:
                volume = v_quad_area(4, coordinates);
                break;
            case VTK_PIXEL:
                Swap3(coordinates, 2, 3);
                volume = v_quad_area(4, coordinates);
                break;
            case VTK_VOXEL:
                Swap3(coordinates, 2, 3);
                Swap3(coordinates, 6, 7);
                volume = v_hex_volume(8, coordinates);
                break;
            case VTK_HEXAHEDRON:
                volume = v_hex_volume(8, coordinates);
                break;
            case VTK_TETRA:
                volume = v_tet_volume(4, coordinates);
                break;
            case VTK_WEDGE:
                double tet_coordinates[4][3];
                volume = 0;
                for (int i = 0 ; i < 3 ; i++)
                {
                    for (int j = 0 ; j < 4 ; j++)
                        for (int k = 0 ; k < 3 ; k++)
                            tet_coordinates[j][k] = coordinates[subdiv[i][j]][k];
                    volume += v_tet_volume(4, tet_coordinates);
                }
                break;
            case VTK_PYRAMID:
                double one[4][3];
                double two[4][3];
                Copy3(coordinates,one[0], 0);
                Copy3(coordinates,one[1], 1);
                Copy3(coordinates,one[2], 2);
                Copy3(coordinates,one[3], 4);
                Copy3(coordinates,two[0], 0);
                Copy3(coordinates,two[1], 2);
                Copy3(coordinates,two[2], 3);
                Copy3(coordinates,two[3], 4);
                volume = v_tet_volume(4,one) + v_tet_volume(4, two);
                break;
            default:
                debug4 << "Cannot calculate volume for cell of type: "
                       << cell->GetCellType() << std::endl
                       << "Extrinsic scalars may not be accurately remapped." << std::endl;
                break;
        } // End switch
        
        volumeArray->SetComponent(n, 0, volume); // store the volume in our array
    }
    
    return volumeArray;
}


// ****************************************************************************
//  Method: avtRemapFilter::GetBounds
//
//  Purpose:
//      Obtain the bounds from the data or from the attributes, depending on
//      useExtents.
//
//  Programmer: rusu1
//  Creation:   Wed Apr  3 13:52:34 PDT 2019
//
//  Modifications:
//      Eddie Rusu, Thu Jul 16 11:09:52 PDT 2020
//      GetSpatialExtents uses +-DBL_MAX as initial placeholders while looking
//      for the extents. If they are not found then the rGridBounds will contain
//      +-DBL_MAX as inputs. So I check for this to ensure 3D determination is
//      correct.
//
// ****************************************************************************
void
avtRemapFilter::GetBounds()
{
    debug4 << "avtRemapFilter::GetBounds" << std::endl;
    if (!atts.GetUseExtents())
    {
        debug5 << "Getting bounds from Start and End fields" << std::endl;
        rGridBounds[0] = atts.GetStartX();
        rGridBounds[1] = atts.GetEndX();
        rGridBounds[2] = atts.GetStartY();
        rGridBounds[3] = atts.GetEndY();
        rGridBounds[4] = atts.GetStartZ();
        rGridBounds[5] = atts.GetEndZ();
    }
    else
    {
        debug5 << "Using extents" << std::endl;
        const avtDataAttributes &datts = GetInput()->GetInfo().GetAttributes();
        avtExtents *exts = datts.GetDesiredSpatialExtents();
        if (exts->HasExtents())
        {
            exts->CopyTo(rGridBounds);
        }
        else
        {
            GetSpatialExtents(rGridBounds);
        }
    }
    if ((fabs(rGridBounds[4]) < 1e-100 && fabs(rGridBounds[5]) < 1e-100) ||
        (rGridBounds[4] == +DBL_MAX && rGridBounds[5] == -DBL_MAX))
    {
        debug5 << "2D Remapping" << std::endl;
        is3D = false;
    }
    else
    {
        is3D = atts.GetIs3D();
    }
}

// ****************************************************************************
//  Method: avtRemapFilter::CreateGrid
//
//  Purpose:
//      Creates a rectilinear grid that spans the bounds of the origin dataset
//      or the user-defined extents. Number of cells in each dimension is
//      defined by the user. Supports 2D and 3D grids.
//
//  Arguments:
//      numX      The number of samples in X.
//      numY      The number of samples in Y.
//      numY      The number of samples in Z.
//      minX      The minimum X index.
//      maxX      The maximum X index.
//      minY      The minimum Y index.
//      maxY      The maximum Y index.
//      minY      The minimum Z index.
//      maxY      The maximum Z index.
//
//  Programmer:   rusu1
//  Creation:     Wed Apr  3 13:52:34 PDT 2019
//
// ****************************************************************************

void
avtRemapFilter::CreateGrid(int numX, int numY, int numZ,
        int minX, int maxX, int minY, int maxY, int minZ, int maxZ)
{
    debug4 << "avtRemapFilter::CreateGrid" << std::endl;
    vtkDataArray *xc = NULL;
    vtkDataArray *yc = NULL;
    vtkDataArray *zc = NULL;

    double width  = rGridBounds[1] - rGridBounds[0];
    double height = rGridBounds[3] - rGridBounds[2];
    double depth  = rGridBounds[5] - rGridBounds[4];

    xc = GetCoordinates(rGridBounds[0], width, numX+1, minX, maxX+1);
    yc = GetCoordinates(rGridBounds[2], height, numY+1, minY, maxY+1);
    zc = GetCoordinates(rGridBounds[4], depth, numZ+1, minZ, maxZ+1);

    rg->SetDimensions(maxX-minX+1, maxY-minY+1, maxZ-minZ+1);
    rg->SetXCoordinates(xc);
    xc->Delete();
    rg->SetYCoordinates(yc);
    yc->Delete();
    rg->SetZCoordinates(zc);
    zc->Delete();
}

void
avtRemapFilter::CreateGrid(
        int numX, int numY, int minX, int maxX, int minY, int maxY)
{
    debug4 << "avtRemapFilter::CreateGrid" << std::endl;
    vtkDataArray *xc = NULL;
    vtkDataArray *yc = NULL;

    double width  = rGridBounds[1] - rGridBounds[0];
    double height = rGridBounds[3] - rGridBounds[2];

    xc = GetCoordinates(rGridBounds[0], width, numX+1, minX, maxX+1);
    yc = GetCoordinates(rGridBounds[2], height, numY+1, minY, maxY+1);

    rg->SetDimensions(maxX-minX+1, maxY-minY+1, 1);
    rg->SetXCoordinates(xc);
    xc->Delete();
    rg->SetYCoordinates(yc);
    yc->Delete();
}


// ****************************************************************************
//  Method: avtRemapFilter::GetCoordinates
//
//  Purpose:
//      Creates a coordinates array based on specifications.
//
//  Arguments:
//      start    The start of the coordinates array.
//      length   The length of the coordinates array.
//      numEls   The number of elements in the coordinates array.
//      myStart  The starting point.
//      myStop   The stopping point.
//
//  Returns:     A vtkDataArray* that holds the coordinates.
//
//  Programmer:  rusu1
//  Creation:    Wed Apr  3 13:52:34 PDT 2019
//
// ****************************************************************************

vtkDataArray *
avtRemapFilter::GetCoordinates(double start, double length, int numEls, int myStart, int myStop)
{
    debug4 << "avtRemapFilter::GetCoordinates" << std::endl;
    vtkDoubleArray *rv = vtkDoubleArray::New();

    //
    // Make sure we don't have any degenerate cases here.
    //
    if (length <= 0. || numEls <= 1 || myStart >= myStop)
    {
        debug4 << "Degenerate cell" << std::endl;
        rv->SetNumberOfValues(1);
        rv->SetValue(0, start);
        return rv;
    }

    int realNumEls = myStop - myStart;
    rv->SetNumberOfValues(realNumEls);
    double offset = length / (numEls-1);
    for (int i = myStart ; i < myStop ; i++)
    {
        rv->SetValue(i-myStart, start + i*offset);
    }

    return rv;
}
