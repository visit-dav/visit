// ************************************************************************* //
//                            avtGDALFileFormat.C                            //
// ************************************************************************* //

#include <avtGDALFileFormat.h>

#include <vtkCellData.h>
#include <vtkCellTypes.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>

#include <avtDatabaseMetaData.h>
#include <avtGhostData.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <DebugStream.h>
#include <snprintf.h>
#include <math.h>

const int avtGDALFileFormat::n_zones_per_dom = 20000;
bool avtGDALFileFormat::gdalInit = false;

// ****************************************************************************
// Method: avtGDAL constructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 6 11:54:59 PDT 2005
//
// Modifications:
//
// ****************************************************************************

avtGDALFileFormat::avtGDALFileFormat(const char *filename)
    : avtSTMDFileFormat(&filename, 1), meshInfo()
{
    poDataset = 0;
    xdims = 0, ydims = 0;
    xmin = xmax = 0.f;
    ymin = ymax = 0.f;
    invertYValues = false;
}

// ****************************************************************************
// Method: avtGDALFileFormat::~avtGDALFileFormat
//
// Purpose: 
//   Destructor for avtGDALFileFormat.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 17 13:44:27 PST 2004
//
// Modifications:
//   
// ****************************************************************************

avtGDALFileFormat::~avtGDALFileFormat()
{
    FreeUpResources();
}

// ****************************************************************************
// Method: avtGDALFileFormat::FreeUpResources
//
// Purpose:
//     When VisIt is done focusing on a particular timestep, it asks that
//     timestep to free up any resources (memory, file descriptors) that
//     it has associated with it.  This method is the mechanism for doing
//     that.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 6 11:54:59 PDT 2005
//
// ****************************************************************************

void
avtGDALFileFormat::FreeUpResources(void)
{
    if(poDataset != 0)
    {
        GDALClose(poDataset);
        poDataset = 0;
    }
}

// ****************************************************************************
// Method: avtGDALFileFormat::GetDataset
//
// Purpose: 
//   Opens the dataset and returns a pointer to it.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 8 16:27:44 PST 2005
//
// Modifications:
//   
// ****************************************************************************

GDALDataset *
avtGDALFileFormat::GetDataset()
{
    if(poDataset == 0)
    {
        if(!gdalInit)
        {
            GDALAllRegister();
            gdalInit = true;
        }

        poDataset = (GDALDataset *)GDALOpen(filenames[0], GA_ReadOnly);
        if(poDataset == 0)
        {
            EXCEPTION1(InvalidFilesException, filenames[0]);
        }
    }
    return poDataset;
}

// ****************************************************************************
// Method: avtGDALFileFormat::PopulateDatabaseMetaData
//
// Purpose:
//     This database meta-data object is like a table of contents for the
//     file.  By populating it, you are telling the rest of VisIt what
//     information it can request from you.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 6 11:54:59 PDT 2005
//
// Modifications:
//   Brad Whitlock, Thu Sep 22 16:38:54 PST 2005
//   Fixed calls to log10 so it builds on win32.
//
// ****************************************************************************

void
avtGDALFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    // Set the database comment by initializing the database.
    TRY
    {
        // Try and open the file.
        GetDataset();
    }
    CATCH(InvalidFilesException)
    {
#ifndef MDSERVER
        // Only rethrow the exception on the engine.
        RETHROW;
#endif
    }
    ENDTRY

    // Come up with an informative database comment.
    std::string title;
    title += "Driver: ";
    title += poDataset->GetDriver()->GetDescription();
    title += "/";
    title += poDataset->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME);
    title += "\n";

    char tmp[100];
    SNPRINTF(tmp, 100, "Size is %dx%dx%d\n", 
        poDataset->GetRasterXSize(), poDataset->GetRasterYSize(),
        poDataset->GetRasterCount());
    title += tmp;

    std::string units("pixels");
    bool haveUnits = true;
    if(poDataset->GetProjectionRef()  != NULL)
    {
        title += "Projection is \"";
        title += poDataset->GetProjectionRef();
        title += "\"\n";

        // Look for the units in the projection.
        int pos = title.rfind("UNIT[");
        if(pos == -1)
        {
            haveUnits = poDataset->GetRasterCount() != 1;
        }
        else
        {
            int start = pos + 6;
            int end = title.find("\"", start);
            if(end != -1)
            {
                units = title.substr(start, end-start);
                debug4 << "units: " << units.c_str() << endl;
            }
            else
            {
                haveUnits = false;
                units = "";
            }
        }
    }

    double adfGeoTransform[6];
    if(poDataset->GetGeoTransform(adfGeoTransform) == CE_None)
    {
        SNPRINTF(tmp, 100, "Origin = (%.6f,%.6f)\n",
               adfGeoTransform[0], adfGeoTransform[3]);
        title += tmp;

        SNPRINTF(tmp, 100, "Pixel Size = (%.6f,%.6f)\n",
                adfGeoTransform[1], adfGeoTransform[5]);
        title += tmp;
    }
    md->SetDatabaseComment(title);

    xdims = poDataset->GetRasterXSize();
    ydims = poDataset->GetRasterYSize();

    // Determine extents.
    if(poDataset->GetGeoTransform(adfGeoTransform) == CE_None)
    {
        xmin = adfGeoTransform[0];
        xmax = xmin + adfGeoTransform[1] * float(xdims);
        ymin = adfGeoTransform[3];
        ymax = ymin + adfGeoTransform[5] * float(ydims);
        invertYValues = false;
    }
    else
    {
        xmax = float(xdims);
        ymax = float(ydims);
        invertYValues = true;
    }

    bool addZComponent = poDataset->GetRasterCount() == 1;

    //
    // Figure out a format string for when we need to add WxH.
    //
    char whFormat[20];
    float lxd = log10((float)xdims);
    float lyd = log10((float)ydims);
    int ndigX = int(lxd) + 1;
    int ndigY = int(lyd) + 1;
    int ndigits = (ndigX > ndigY) ? ndigX : ndigY;
    SNPRINTF(whFormat, 20, "%%s%%0%ddx%%0%dd", ndigits, ndigits);

    //
    // Determine how many resolutions we should make
    //
    int xsize = xdims;
    int ysize = ydims;
    int resolution = 0;
    char meshName[200];
    char elevatedMeshName[200];
    MeshInfo newMesh, elevatedMesh;
    int scale = 1;
    do
    {
        newMesh.xdims = xsize;
        newMesh.ydims = ysize;
        CalculateNDomains(xsize, ysize,
                          newMesh.nYPerDomain, newMesh.numDomains);
        newMesh.scale = scale;
        newMesh.hasZComponent = false;

        elevatedMesh.xdims = xsize;
        elevatedMesh.ydims = ysize;
        CalculateNDomains(xsize, ysize,
                          elevatedMesh.nYPerDomain, elevatedMesh.numDomains);
        elevatedMesh.scale = scale;
        elevatedMesh.hasZComponent = true;

        if(resolution == 0)
        {
            SNPRINTF(meshName, 200, "mesh", xsize, ysize);
            meshInfo[meshName] = newMesh;

            if(addZComponent)
            {
                SNPRINTF(elevatedMeshName, 200, "elevated/mesh",
                         xsize, ysize);
                meshInfo[elevatedMeshName] = elevatedMesh;
            }
        }
        else
        {
            SNPRINTF(meshName, 200, whFormat, "lower_res/resolution_", xsize,ysize);
            meshInfo[meshName] = newMesh;

            if(addZComponent)
            {
                SNPRINTF(elevatedMeshName, 200, whFormat,
                         "elevated/lower_res/resolution_", xsize, ysize);
                meshInfo[elevatedMeshName] = elevatedMesh;
            }
        }

        // Add the mesh
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = meshName;
        mesh->meshType = AVT_RECTILINEAR_MESH;
        mesh->numBlocks = newMesh.numDomains;
        mesh->blockOrigin = 1;
        mesh->spatialDimension = 2;
        mesh->topologicalDimension = 2;
        mesh->hasSpatialExtents = true;
        mesh->minSpatialExtents[0] = xmin;
        mesh->minSpatialExtents[1] = ymin;
        mesh->minSpatialExtents[2] = 0.f;
        mesh->maxSpatialExtents[0] = xmax;
        mesh->maxSpatialExtents[1] = ymax;
        mesh->maxSpatialExtents[2] = 0.f;
        if(haveUnits)
        {
            mesh->xUnits = units;
            mesh->yUnits = units;
            mesh->zUnits = units;
        }
        md->Add(mesh);

        // Add the mesh
        if(addZComponent)
        {
            avtMeshMetaData *emesh = new avtMeshMetaData;
            emesh->name = elevatedMeshName;
            emesh->meshType = AVT_UNSTRUCTURED_MESH;
            emesh->numBlocks = elevatedMesh.numDomains;
            emesh->blockOrigin = 1;
            emesh->spatialDimension = 3;
            emesh->topologicalDimension = 2;
            emesh->hasSpatialExtents = false;
            if(haveUnits)
            {
                emesh->xUnits = units;
                emesh->yUnits = units;
                emesh->zUnits = units;
            }
            md->Add(emesh);
        }

        xsize /= 2;
        ysize /= 2;
        scale *= 2;
        ++resolution;
    } while(newMesh.numDomains > 1);

    bool haveRed = false, haveGreen = false, haveBlue = false;
    if(meshInfo.size() == 1)
    {
        // Add the variables.
        for(int channel = 0; channel < poDataset->GetRasterCount(); ++channel)
        {
            GDALRasterBand  *poBand = poDataset->GetRasterBand(channel + 1);
            std::string cn(GetComponentName(GDALGetColorInterpretationName(
                poBand->GetColorInterpretation())));

            avtScalarMetaData *smd = new avtScalarMetaData(cn,
                        "mesh", AVT_ZONECENT);
            if(cn == "height")
            {
                smd->units = units;
                smd->hasUnits = haveUnits;
            }
            else if(haveUnits)
            {
                smd->units = "color";
                smd->hasUnits = true;
            }
            md->Add(smd);

            haveRed   |= cn == "red";
            haveGreen |= cn == "green";
            haveBlue  |= cn == "blue";
        }

        if(haveRed && haveGreen && haveBlue)
        {
            AddScalarVarToMetaData(md, "intensity", "mesh", AVT_ZONECENT, NULL);
            AddVectorVarToMetaData(md, "color", "mesh", AVT_ZONECENT, 4);
        }
    }
    else
    {
        for(MeshInfoMap::const_iterator pos = meshInfo.begin();
            pos != meshInfo.end(); ++pos)
        {
            char vn[200];

            // Add the variables.
            avtCentering centering = pos->second.hasZComponent ? AVT_NODECENT :
                AVT_ZONECENT;
            bool topLevelMesh = pos->first == "mesh";
            for(int channel = 0; channel < poDataset->GetRasterCount(); ++channel)
            {
                GDALRasterBand  *poBand = poDataset->GetRasterBand(channel + 1);
                std::string cn(GetComponentName(GDALGetColorInterpretationName(
                     poBand->GetColorInterpretation())));
                if(topLevelMesh)
                    SNPRINTF(vn, 200, "%s", cn.c_str());
                else
                    SNPRINTF(vn, 200, "%s/%s", pos->first.c_str(), cn.c_str());

                avtScalarMetaData *smd = new avtScalarMetaData(vn,
                            pos->first, centering);
                if(cn == "height")
                {
                    smd->units = units;
                    smd->hasUnits = haveUnits;
                }
                else if(haveUnits)
                {
                    smd->units = "color";
                    smd->hasUnits = true;
                }
                md->Add(smd);

                haveRed   |= cn == "red";
                haveGreen |= cn == "green";
                haveBlue  |= cn == "blue";
            }

            if(haveRed && haveGreen && haveBlue)
            {
                if(topLevelMesh)
                    SNPRINTF(vn, 200, "%s", "intensity");
                else
                    SNPRINTF(vn, 200, "%s/intensity", pos->first.c_str());
                AddScalarVarToMetaData(md, vn, pos->first, centering, NULL);
                if(topLevelMesh)
                    SNPRINTF(vn, 200, "%s", "color");
                else
                    SNPRINTF(vn, 200, "%s/color", pos->first.c_str());
                AddVectorVarToMetaData(md, vn, pos->first, centering, 4);
            }
        }
    }
}

// ****************************************************************************
// Method: avtGDALFileFormat::CalculateNDomains
//
// Purpose: 
//   Calculate the number of domains that a grid xsize*ysize should be broken
//   into.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 8 16:28:07 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtGDALFileFormat::CalculateNDomains(int xsize, int ysize, int &nY, int &ndoms)
{
    nY = 1;
    for( ; nY * xsize < n_zones_per_dom; ++nY);
    
    int nzones = xsize * ysize;
    ndoms = nzones / (nY * xsize);
    if(ndoms * (nY * xsize) < nzones)
        ++ndoms;
}

// ****************************************************************************
// Method: avtGDALFileFormat::GetMesh
//
// Purpose:
//     Gets the mesh associated with this file.  The mesh is returned as a
//     derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//     vtkUnstructuredGrid, etc).
//
// Arguments:
//    domain   : The domain whose data we want.
//    meshname : The name of the mesh of interest.  This can be ignored if
//               there is only one mesh.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 6 11:54:59 PDT 2005
//
// Modifications:
//
// ****************************************************************************

vtkDataSet *
avtGDALFileFormat::GetMesh(int domain, const char *meshname)
{
#ifdef MDSERVER
    return 0;
#else
    const char *mName = "avtGDALFileFormat::GetMesh: ";

    MeshInfoMap::const_iterator pos = meshInfo.find(meshname);
    if(pos == meshInfo.end())
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }

    vtkDataSet *retval = 0;
    if(pos->second.hasZComponent)
        retval = CreateElevatedMesh(pos->second, domain, meshname);
    else
        retval = CreateFlatMesh(pos->second, domain, meshname);

    return retval;
#endif
}

// ****************************************************************************
// Method: avtGDALFileFormat::CreateCoordinates
//
// Purpose: 
//   Creates coordinate arrays that we use for the mesh.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 8 16:29:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
avtGDALFileFormat::CreateCoordinates(const avtGDALFileFormat::MeshInfo &info,
    int domain, vtkFloatArray **coords, int size_offset)
{
    const char *mName = "avtGDALFileFormat::CreateCoordinates: ";
    int nCellsInX = info.xdims;
    int nCellsInY = info.ydims;
    int nYPerDomain = info.nYPerDomain;
    int numDomains = info.numDomains;

    //
    // Create the entire Y-coordinate array
    //
    double y0 = ymin;
    double y1 = ymax;
    if(invertYValues)
    {
        y0 = ymax;
        y1 = ymin;
    }
    double *Y = new double[nCellsInY+1];
    int i;
    for(i = 0; i < nCellsInY+1; ++i)
    {
        double t = double(i) / double(nCellsInY);
        Y[i] = y0 * (1. - t) + y1 * t;
    }

    // We have to cut up y.
    int minYIndex = domain * nYPerDomain;
    int maxYIndex = minYIndex + nYPerDomain;
    if(maxYIndex >= nCellsInY-1)
        maxYIndex = nCellsInY;
    int nRealCells = (maxYIndex - minYIndex) * nCellsInX;

    // Add a row of ghost zones on the bottom.
    if(domain > 0)
    {
        --minYIndex;
        debug4 << mName << "Adding a row of ghost zones on bottom." << endl;
    }
    // Add a row of ghost zones on the top.
    if(domain < numDomains-1)
    {
        ++maxYIndex;
        debug4 << mName << "Adding a row of ghost zones on top." << endl;
    }

    int size = maxYIndex - minYIndex;
    int dims[3] = {nCellsInX+size_offset, size+size_offset, 1};
    coords[0] = vtkFloatArray::New();
    coords[1] = vtkFloatArray::New();
    coords[2] = vtkFloatArray::New();

    debug4 << mName << "domain " << domain
           << "size+1=" << size+1
           << " minYIndex=" << minYIndex
           << " maxYIndex=" << maxYIndex
           << " nRealCells=" << nRealCells
           << endl;

    // Populate X.
    coords[0]->SetNumberOfTuples(dims[0]);
    for (i = 0; i < dims[0]; ++i)
    {
        double t = double(i) / double(dims[0]-1);
        double val = (t * xmax) + ((1. - t) * xmin);
        coords[0]->SetComponent(i, 0, float(val));
    }

    // Populate Y.
    coords[1]->SetNumberOfTuples(dims[1]);
    int index = 0;
    for (i = minYIndex; i < maxYIndex+size_offset; ++i, ++index)
        coords[1]->SetComponent(index, 0, Y[i]);

    // Populate Z.
    coords[2]->SetNumberOfTuples(1);
    coords[2]->SetComponent(0, 0, 0.);

    delete [] Y;
}

// ****************************************************************************
// Method: avtGDALFileFormat::CreateFlatMesh
//
// Purpose: 
//   Creates the flat 2D mesh.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 8 16:29:35 PST 2005
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
avtGDALFileFormat::CreateFlatMesh(const avtGDALFileFormat::MeshInfo &info, int domain,
    const char *meshname)
{
#ifdef MDSERVER
    return 0;
#else
    const char *mName = "avtGDALFileFormat::CreateFlatMesh: ";

    vtkFloatArray *coords[3] = {0,0,0};
    CreateCoordinates(info, domain, coords, 1);

    int dims[3];
    dims[0] = coords[0]->GetNumberOfTuples();
    dims[1] = coords[1]->GetNumberOfTuples();
    dims[2] = coords[2]->GetNumberOfTuples();

    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New(); 
    rgrid->SetDimensions(dims);
    rgrid->SetXCoordinates(coords[0]);
    coords[0]->Delete();
    rgrid->SetYCoordinates(coords[1]);
    coords[1]->Delete();
    rgrid->SetZCoordinates(coords[2]);
    coords[2]->Delete();

    int nCellsInX = dims[0] - 1;
    int nCellsInY = dims[1] - 1;
    int nRealCellsInY = nCellsInY;
    if(domain > 0)
        --nRealCellsInY;
    if(domain < info.numDomains-1)
        --nRealCellsInY;
    int nRealCells = nCellsInX * nRealCellsInY;

    //
    // Set up the ghost zones array.
    //
    vtkUnsignedCharArray *ghostZones = CreateGhostZonesArray(nCellsInX,
        nRealCells, domain, info.numDomains);
    rgrid->GetCellData()->AddArray(ghostZones);
    ghostZones->Delete();

    return rgrid;
#endif
}

// ****************************************************************************
// Method: avtGDALFileFormat::CreateGhostZonesArray
//
// Purpose: 
//   Create ghost zones for the flat mesh.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 8 16:29:50 PST 2005
//
// Modifications:
//   
// ****************************************************************************

vtkUnsignedCharArray *
avtGDALFileFormat::CreateGhostZonesArray(int nCellsInX, 
    int nRealCells, int domain, int numDomains)
{
    // Figure out the number of cells.
    int totalCells = nRealCells;
    if(domain > 0)
        totalCells += nCellsInX;
    if(domain < numDomains-1)
        totalCells += nCellsInX;

    vtkUnsignedCharArray *ghostZones = vtkUnsignedCharArray::New();
    ghostZones->SetName("avtGhostZones");
    ghostZones->SetNumberOfTuples(totalCells);
    unsigned char *gv = ghostZones->GetPointer(0);
    unsigned char realVal = 0, ghostVal = 0;
    avtGhostData::AddGhostZoneType(ghostVal, DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);

    if(domain > 0)
    {
        for(int cell = 0; cell < nCellsInX; ++cell)
            *gv++ = ghostVal;
    }
    for(int cell = 0; cell < nRealCells; ++cell)
        *gv++ = realVal;
    if(domain < numDomains-1)
    {
        for(int cell = 0; cell < nCellsInX; ++cell)
            *gv++ = ghostVal;
    }

    return ghostZones;
}

// ****************************************************************************
// Method: avtGDALFileFormat::CreateElevatedMesh
//
// Purpose: 
//   Create the elevated mesh.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 8 16:30:04 PST 2005
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
avtGDALFileFormat::CreateElevatedMesh(const avtGDALFileFormat::MeshInfo &info,
    int domain, const char *meshname)
{
    const char *mName = "avtGDALFileFormat::CreateElevatedMesh: ";
    GetDataset();

    // Read in the height values for this domain.
    vtkFloatArray *heights = ReadVar(info, poDataset->GetRasterBand(1),
                                     domain);
    debug4 << mName << "heights array has " << heights->GetNumberOfTuples()
           << " tuples" << endl;

    // Create the X, Y coordinates.
    vtkFloatArray *coords[3] = {0,0,0};
    CreateCoordinates(info, domain, coords, 0);

    int nxnodes = coords[0]->GetNumberOfTuples();
    int nynodes = coords[1]->GetNumberOfTuples();
    debug4 << mName << "Coordinates have [" << nxnodes << ", " << nynodes
           << "] values" << endl;
    float *xc = (float *)coords[0]->GetVoidPointer(0);
    float *yc = (float *)coords[1]->GetVoidPointer(0);
    float *zc = (float *)heights->GetVoidPointer(0);
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(nynodes * nxnodes);
    vtkIdType id = 0;
    for(int j = 0; j < nynodes; ++j)
    {
        xc = (float *)coords[0]->GetVoidPointer(0);
        for(int i = 0; i < nxnodes; ++i)
        {
            float pt[3];
            pt[0] = *xc++;
            pt[1] = *yc;
            pt[2] = *zc++;
            pts->SetPoint(id++, pt);
        }
        ++yc;
    }

    int nxcells = nxnodes-1;
    int nycells = nynodes-1;
    vtkPolyData *pd = vtkPolyData::New();
    pd->SetPoints(pts);
    pd->Allocate(nycells * nxcells);
    vtkIdType verts[4];
    for(int j = 0; j < nycells; ++j)
    {
        for(int i = 0; i < nxcells; ++i)
        {
            verts[0] = j * nxnodes + i;
            verts[1] = j * nxnodes + i + 1;
            verts[2] = (j+1) * nxnodes + i + 1;
            verts[3] = (j+1) * nxnodes + i;
            pd->InsertNextCell(VTK_QUAD, 4, verts);
        }
    }

    // Delete the coords.
    coords[0]->Delete();
    coords[1]->Delete();
    coords[2]->Delete();
    heights->Delete();

#if 1
    // Add ghost nodes.
    int nnodes = nxnodes * nynodes;
    vtkUnsignedCharArray *ghost_nodes = vtkUnsignedCharArray::New();
    ghost_nodes->SetName("avtGhostNodes");
    ghost_nodes->SetNumberOfTuples(nnodes);
    unsigned char *gn = (unsigned char *)ghost_nodes->GetVoidPointer(0);
    unsigned char ghostVal;
    avtGhostData::AddGhostNodeType(ghostVal, DUPLICATED_NODE);
    int nrealnodes = nnodes;
    if(domain > 0)
        nrealnodes -= nxnodes;
    if(domain < info.numDomains-1)
        nrealnodes -= nxnodes;

    int i;
    if(domain > 0)
    {
        for(i = 0; i < nxnodes; ++i)
             *gn++ = ghostVal;
    }
    for(i = 0; i < nrealnodes; ++i)
        *gn++ = 0;
    if(domain < info.numDomains-1)
    {
        for(i = 0; i < nxnodes; ++i)
            *gn++ = ghostVal;
    }
    pd->GetPointData()->AddArray(ghost_nodes);
    ghost_nodes->Delete();
#endif
    return pd;
}

// ****************************************************************************
// Method: avtGDALFileFormat::GetVar
//
// Purpose:
//     Gets a scalar variable associated with this file. 
//
// Arguments:
//    domain  : The domain whose data we want.
//    varname : The name of the variable requested.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 6 11:54:59 PDT 2005
//
// Modifications:
//
// ****************************************************************************

vtkDataArray *
avtGDALFileFormat::GetVar(int domain, const char *varname)
{
#ifdef MDSERVER
    return 0;
#else
    const char *mName = "avtGDALFileFormat::GetVar: ";

    // Make sure that we can read the dataset.
    GetDataset();
   
    std::string meshName(varname);
    std::string Varname(varname);
    int index = meshName.rfind("/");
    if(index != -1)
    {
        meshName = meshName.substr(0, index);
        Varname = Varname.substr(index+1, Varname.size()-index);
    }
    else
        meshName = "mesh";
    debug4 << mName << "varname=" << varname
           << ", meshName=" << meshName.c_str()
           << ", Varname=" << Varname.c_str() << endl;

    MeshInfoMap::const_iterator pos = meshInfo.find(meshName);
    if(pos == meshInfo.end())
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    vtkFloatArray *arr = 0;
    if(Varname == "intensity")
    {
        vtkFloatArray *ch[3] = {0,0,0};
        for(int channel = 0; channel < poDataset->GetRasterCount(); ++channel)
        {
            GDALRasterBand  *poBand = poDataset->GetRasterBand(channel + 1);
            ch[channel] = ReadVar(pos->second, poBand, domain);
        }

        int n = ch[0]->GetNumberOfTuples();
        arr = vtkFloatArray::New();
        arr->SetNumberOfTuples(n);
        float *r = (float *)ch[0]->GetVoidPointer(0);
        float *g = (float *)ch[1]->GetVoidPointer(0);
        float *b = (float *)ch[2]->GetVoidPointer(0);
        float *vals = (float *)arr->GetVoidPointer(0);
        for(int i = 0; i < n; ++i)
        {
            *vals++ = (*r + *g + *b) / 3.f;
            ++r; ++g; ++b;
        }

        ch[0]->Delete();
        ch[1]->Delete();
        ch[2]->Delete();
    }
    else
    {
        // Determine the channel number
        int chno = -1;
        for(int channel = 0; channel < poDataset->GetRasterCount(); ++channel)
        {
            GDALRasterBand  *poBand = poDataset->GetRasterBand(channel + 1);
            if(GetComponentName(GDALGetColorInterpretationName(poBand->GetColorInterpretation()))
               == Varname)
            {
                return ReadVar(pos->second, poBand, domain);
            }
        }
    }

    return arr;
#endif
}

// ****************************************************************************
// Method: avtGDALFileFormat::GetVectorVar
//
// Purpose: 
//   Read in the color variable if it exists.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 8 16:30:21 PST 2005
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
avtGDALFileFormat::GetVectorVar(int domain, const char *varname)
{
#ifdef MDSERVER
    return 0;
#else
    const char *mName = "avtGDALFileFormat::GetVectorVar: ";

    // Make sure that we can read the dataset.
    GetDataset();
   
    std::string meshName(varname);
    std::string Varname(varname);
    int index = meshName.rfind("/");
    if(index != -1)
    {
        meshName = meshName.substr(0, index);
        Varname = Varname.substr(index+1, Varname.size()-index);
    }
    else
        meshName = "mesh";
    debug4 << mName << "varname=" << varname
           << ", meshName=" << meshName.c_str()
           << ", Varname=" << Varname.c_str() << endl;

    MeshInfoMap::const_iterator pos = meshInfo.find(meshName);
    if(pos == meshInfo.end())
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    vtkFloatArray *cvar = 0;
    if(Varname == "color")
    {
        if(poDataset->GetRasterCount() == 3)
        {
            vtkFloatArray *ch[3] = {0,0,0};
            for(int channel = 0; channel < poDataset->GetRasterCount(); ++channel)
            {
                GDALRasterBand  *poBand = poDataset->GetRasterBand(channel + 1);
                ch[channel] = ReadVar(pos->second, poBand, domain);
            }

            int n = ch[0]->GetNumberOfTuples();
            cvar = vtkFloatArray::New();
            cvar->SetNumberOfComponents(4);
            cvar->SetNumberOfTuples(n);
            float *r = (float *)ch[0]->GetVoidPointer(0);
            float *g = (float *)ch[1]->GetVoidPointer(0);
            float *b = (float *)ch[2]->GetVoidPointer(0);
            float *vals = (float *)cvar->GetVoidPointer(0);
            for(int i = 0; i < n; ++i)
            {
                *vals++ = *r++;
                *vals++ = *g++;
                *vals++ = *b++;
                *vals++ = 255.;
            }

            ch[0]->Delete();
            ch[1]->Delete();
            ch[2]->Delete();
        }
        else
        {
            vtkFloatArray *ch[4] = {0,0,0};
            for(int channel = 0; channel < 4; ++channel)
            {
                GDALRasterBand  *poBand = poDataset->GetRasterBand(channel + 1);
                ch[channel] = ReadVar(pos->second, poBand, domain);
            }

            int n = ch[0]->GetNumberOfTuples();
            cvar = vtkFloatArray::New();
            cvar->SetNumberOfComponents(4);
            cvar->SetNumberOfTuples(n);
            float *r = (float *)ch[0]->GetVoidPointer(0);
            float *g = (float *)ch[1]->GetVoidPointer(0);
            float *b = (float *)ch[2]->GetVoidPointer(0);
            float *a = (float *)ch[3]->GetVoidPointer(0);
            float *vals = (float *)cvar->GetVoidPointer(0);
            for(int i = 0; i < n; ++i)
            {
                *vals++ = *r++;
                *vals++ = *g++;
                *vals++ = *b++;
                *vals++ = *a++;
            }

            ch[0]->Delete();
            ch[1]->Delete();
            ch[2]->Delete(); 
            ch[3]->Delete(); 
        }
    }
    else
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    return cvar;
#endif
}

// ****************************************************************************
// Method: avtGDALFileFormat::ReadVar
//
// Purpose: 
//   Reads data for a color channel in a specified domain.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 8 16:30:42 PST 2005
//
// Modifications:
//   
// ****************************************************************************

vtkFloatArray *
avtGDALFileFormat::ReadVar(const avtGDALFileFormat::MeshInfo &info,
    GDALRasterBand *poBand, int domain)
{
    const char *mName = "avtGDALFileFormat::ReadVar: ";
    int nCellsInX = info.xdims;
    int nCellsInY = info.ydims;
    int nYPerDomain = info.nYPerDomain;
    int numDomains = info.numDomains;

    // Determine the size of the window into which we'll read the data.
    int xSampleSize = info.xdims;
    int ySampleSize = info.nYPerDomain;
    if(domain == info.numDomains-1)
        ySampleSize = info.ydims - info.nYPerDomain * domain;
    if(domain > 0)
        ++ySampleSize;
    if(domain < numDomains-1)
        ++ySampleSize;

    debug4 << mName << "domain=" << domain
           << ", nYPerDomain=" << nYPerDomain
           << ", info.scale=" << info.scale
           << ", info.xdims=" << info.xdims
           << ", info.ydims=" << info.ydims
           << ", xdims=" << xdims
           << ", ydims=" << ydims << endl;

    // Come up with the y offset and size of the window in the entire image.
    int x = 0;
    int y = domain * nYPerDomain * info.scale;
    int xsize = xdims;
    int ysize = nYPerDomain * info.scale;
    if(y + ysize >= ydims)
    {
        ysize = (ydims - y);
    }

    // Get ghost data for the bottom.
    if(domain > 0)
    {
        y -= info.scale;
        ysize += info.scale;
    }
    // Get ghost data for the top.
    if(domain < numDomains-1)
    {
        ysize += info.scale;
    }

    // Read a block of floats into a new VTK array.
    int nxy = xSampleSize * ySampleSize;
    vtkFloatArray *f = vtkFloatArray::New();
    f->SetNumberOfTuples(nxy);
    float *dest = (float *)f->GetVoidPointer(0);
    debug4 << "Read ["
           << x << ", " << y << ", "
           << xsize << ", " << ysize << "] into float["
           << xSampleSize << ", " << ySampleSize << "]" << endl;
    poBand->RasterIO( GF_Read, x, y, xsize, ysize,
              dest, xSampleSize, ySampleSize, GDT_Float32, 
              0, 0 );

    // Zero out any numbers below the min.
    int haveMin = 0;
    int haveMax = 0;
    double MinMax[2] = {0., 0.};
    MinMax[0] = poBand->GetMinimum(&haveMin);
    MinMax[1] = poBand->GetMaximum(&haveMax);
    if(haveMin == 0)
        GDALComputeRasterMinMax((GDALRasterBandH)poBand, TRUE, MinMax);
    if(haveMin != 0)
        debug4 << mName << "Min: " << MinMax[0] << endl;
    if(haveMax != 0)
        debug4 << mName << "Max: " << MinMax[1] << endl;
    float fmin = float(MinMax[0]);
    dest = (float *)f->GetVoidPointer(0);
    for(int i = 0; i < nxy; ++i)
    {
        if(*dest < fmin)
            *dest = fmin;
        ++dest;
    }

    return f;
}

// ****************************************************************************
// Method: avtGDALFileFormat::GetComponentName
//
// Purpose: 
//   Returns the name of the variable that we'll serve up to the user.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 8 16:31:07 PST 2005
//
// Modifications:
//   
// ****************************************************************************

std::string
avtGDALFileFormat::GetComponentName(const char *name) const
{
    std::string retval(name);
    if(retval == "Undefined")
        retval = "height";
    else if(retval == "Red")
        retval = "red";
    else if(retval == "Green")
        retval = "green";
    else if(retval == "Blue")
        retval = "blue";
    else if(retval == "Alpha")
        retval = "alpha";
    return retval;
}
