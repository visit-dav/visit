// ************************************************************************* //
//                            avtImageFileFormat.C                           //
// ************************************************************************* //

#include <avtDataSelection.h>
#include <avtImageFileFormat.h>
#include <avtLogicalSelection.h>
#include <avtSpatialBoxSelection.h>

#include <string>
#include <visitstream.h>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#ifndef MDSERVER
#include <vtkExtractVOI.h>
#include <vtkPNMReader.h>
#include <vtkPNGReader.h>
#include <vtkJPEGReader.h>
#include <vtkTIFFReader.h>
#include <vtkBMPReader.h>
#endif

#include <avtDatabaseMetaData.h>

#include <InvalidVariableException.h>

#include <BadIndexException.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <vtkCellType.h>
#include <vtkCellData.h>
#include <vector>
using     std::vector;
using     std::string;

// ****************************************************************************
//  Method: avtImage constructor
//
//  Programmer: Chris Wojtan
//  Creation:   Thu Jun 3 09:50:31 PDT 2004
//
//  Modifications: 
//
//     Chris Wojtan, Mon Jun 21 15:36 PDT 2004
//     Moved most of the Image loading code into ReadInImage() instead of the constructor
//
// ****************************************************************************

avtImageFileFormat::avtImageFileFormat(const char *filename)
    : avtSTSDFileFormat(filename)
{

    // load in the image info
    fname = filename;
    image = NULL;
    readInImage = false;
}


// ***************************************************************************
//  Method: avtImageFileFormat destructor
//
//  Programmer: Chris Wojtan
//  Creation:   June 6, 2004
//
// **************************************************************************

avtImageFileFormat::~avtImageFileFormat()
{
    if(image != NULL)
    {
        image->Delete();
        image = NULL;
    }
    
    cellvars.clear();
    cellvarnames.clear();
    pointvars.clear();
    pointvarnames.clear();
}

// ***************************************************************************
//  Method: RegisterDataSelections 
//
//  Programmer: Mark C. Miller 
//  Creation:   November 3, 2004 
//
// **************************************************************************

void
avtImageFileFormat::RegisterDataSelections(
    const vector<avtDataSelection_p> &sels,
    vector<bool> *selectionsApplied)
{
    selList     = sels;
    selsApplied = selectionsApplied;
}

// ***************************************************************************
//  Method: ProcessSelections 
//
//  Purpose: Walk through all the data selections, decide which we can
//  service here and compose all that we can service into a single, logical
//  selection returned in the argument.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 3, 2004 
//
// **************************************************************************

bool
avtImageFileFormat::ProcessDataSelections(int *xmin, int *xmax,
                                          int *ymin, int *ymax)
{
    bool retval = false;

    avtLogicalSelection composedSel;
    for (int i = 0; i < selList.size(); i++)
    {
        if (string(selList[i]->GetType()) == "Logical Data Selection")
        {
            avtLogicalSelection *sel = (avtLogicalSelection *) *(selList[i]);
            int strides[3];
            sel->GetStrides(strides);

            // currently handles only stride 1 
            bool stridesOk = true;
            for (int j = 0; j < 3; j++)
            {
                if (strides[j] != 1)
                {
                    stridesOk = false;
                    break;
                }
            }

            if (stridesOk == false)
            {
                (*selsApplied)[i] = false;
                continue;
            }

            // overrwrite method-scope arrays with the new indexing
            composedSel.Compose(*sel);
            (*selsApplied)[i] = true;
            retval = true;

        }
        else if (string(selList[i]->GetType()) == "Spatial Box Data Selection")
        {
            avtSpatialBoxSelection *sel = (avtSpatialBoxSelection *) *(selList[i]);

            float mins[3], maxs[3];
            sel->GetMins(mins);
            sel->GetMaxs(maxs);
            avtSpatialBoxSelection::InclusionMode imode =
                sel->GetInclusionMode();

            // we won't handle clipping of zones here
            if ((imode != avtSpatialBoxSelection::Whole) &&
                (imode != avtSpatialBoxSelection::Partial))
            {
                (*selsApplied)[i] = false;
                continue;
            }

            int imins[3], imaxs[3];
            for (int j = 0; j < 3; j++)
            {
                int imin = (int) mins[j];
                if (((float) imin < mins[j]) &&
                    (imode == avtSpatialBoxSelection::Whole))
                    imin++;
                
                int imax = (int) maxs[j];
                if (((float) imax < maxs[j]) &&
                    (imode == avtSpatialBoxSelection::Partial))
                    imax++;

                imins[j] = imin;
                imaxs[j] = imax;
            }

            avtLogicalSelection newSel;
            newSel.SetStarts(imins);
            newSel.SetStops(imaxs);

            composedSel.Compose(newSel);
            (*selsApplied)[i] = true;
            retval = true;
        }
        else
        {
            // indicate we won't handle this selection
            (*selsApplied)[i] = false;
        }
    }

    int mins[3], maxs[3];
    composedSel.GetStarts(mins);
    composedSel.GetStops(maxs);

    *xmin = mins[0]; 
    *xmax = maxs[0]; 
    *ymin = mins[1]; 
    *ymax = maxs[1]; 

    return retval;
}

// *****************************************************************************
//  Method: avtImageVileFormat::ReadInImage
//
//  Purpose:
//      Reads in the image.
//
//  Programmer: Chris Wojtan
//  Creation:   June 6, 2004
//
//  Modifications:
//
//    Chris Wojtan Jun 21, 15:38 PDT 2004
//    Moved most of the loading code into this function in order to speed up
//    the constructor
//
//    Jeremy Meredith, Wed Jul  7 13:49:18 PDT 2004
//    The MDServer doesn't need to read the images; it is making assumptions
//    about what is in the file already for speed purposes, and this lets us
//    get away with not using the extra VTK libraries.  Also commented out
//    the #include files at the top of the file.  Also, there was no case
//    for monochrome images, so I added one, and I threw an exception on an
//    unexpected number of channels.  And, made intensity be the numerical
//    average of the RGB channels instead of the sum.
//
//    Mark C. Miller, Thu Nov  4 18:21:25 PST 2004
//    Add some support for data selections. Not all formats can perform
//    selection during read. Those that can do. Those that cannot read the
//    whole image and then extract the region of interest after the read.
//    This is still useful to VisIt as it can reduce the amount of data
//    the engine keeps around
//
// *****************************************************************************

void avtImageFileFormat::ReadInImage(void)
{
#ifdef MDSERVER
    return;
#else
    if(image != NULL)
    {
        image->Delete();
    }

    // find the file extension
    int i, start;
    for(i=0; i<fname.size(); i++)
        if(fname[i] == '.')
            start = i;
    string ext(fname, start+1, fname.size()-1);

    // process any data selections we can handle, here
    int xmin, xmax, ymin, ymax;
    bool haveSelections = ProcessDataSelections(&xmin, &xmax, &ymin, &ymax);

    // select the appropriate reader for the file extension
    if((ext == "pnm")||(ext == "PNM")||(ext == "ppm")||(ext == "PPM"))
    {
        vtkPNMReader *reader = vtkPNMReader::New();
        if (haveSelections)
            reader->SetDataVOI(xmin,xmax,ymin,ymax,0,0);
        reader->SetFileName(filename);
        image = reader->GetOutput();
        image->Register(NULL);
        image->Update();
        image->SetSource(NULL);
        reader->Delete();
    }
    else if((ext == "png")||(ext == "PNG"))
    {
        vtkPNGReader *reader = vtkPNGReader::New();
        vtkExtractVOI *extvoi = vtkExtractVOI::New();
        reader->SetFileName(filename);
        if (haveSelections)
        {
            extvoi->SetVOI(xmin,xmax,ymin,ymax,0,0);
            extvoi->SetInput(reader->GetOutput());
            image = extvoi->GetOutput();
        }
        else
        {
           image = reader->GetOutput();
        }
        image->Register(NULL);
        image->Update();
        image->SetSource(NULL);
        reader->Delete();
        extvoi->Delete();
    }
    else if((ext == "jpg")||(ext == "jpeg")||(ext == "JPG")||(ext == "JPEG"))
    {
        vtkJPEGReader *reader = vtkJPEGReader::New();
        vtkExtractVOI *extvoi = vtkExtractVOI::New();
        reader->SetFileName(filename);
        if (haveSelections)
        {
            extvoi->SetVOI(xmin,xmax,ymin,ymax,0,0);
            extvoi->SetInput(reader->GetOutput());
            image = extvoi->GetOutput();
        }
        else
        {
           image = reader->GetOutput();
        }
        image->Register(NULL);
        image->Update();
        image->SetSource(NULL);
        reader->Delete();
        extvoi->Delete();
    }
    else if((ext == "tif")||(ext == "tiff")||(ext == "TIF")||(ext == "TIFF"))
    {
        vtkTIFFReader *reader = vtkTIFFReader::New();
        vtkExtractVOI *extvoi = vtkExtractVOI::New();
        reader->SetFileName(filename);
        if (haveSelections)
        {
            extvoi->SetVOI(xmin,xmax,ymin,ymax,0,0);
            extvoi->SetInput(reader->GetOutput());
            image = extvoi->GetOutput();
        }
        else
        {
           image = reader->GetOutput();
        }
        image->Register(NULL);
        image->Update();
        image->SetSource(NULL);
        reader->Delete();
        extvoi->Delete();
    }
    else if((ext == "bmp")||(ext == "BMP"))
    {
        vtkBMPReader *reader = vtkBMPReader::New();
        if (haveSelections)
            reader->SetDataVOI(xmin,xmax,ymin,ymax,0,0);
        reader->SetFileName(filename);
        image = reader->GetOutput();
        image->Register(NULL);
        image->Update();
        image->SetSource(NULL);
        reader->Delete();
    }
    else
        EXCEPTION1(InvalidFilesException, fname.c_str());

    int dims[3];
    image->GetDimensions(dims);
    int xdim = dims[0];
    int ydim = dims[1];
    
    // load in color info
    std::vector<float> red, green, blue, intensity, alpha;
    red.resize(xdim*ydim);
    green.resize(xdim*ydim);
    blue.resize(xdim*ydim);
    alpha.resize(xdim*ydim);

    // if the image has an alpha channel set, read it in
    if (image->GetNumberOfScalarComponents() == 4)
    {
        for(int j=0; j < ydim; j++)
        {
            for(i=0; i < xdim; i++)
            {
                red[j*xdim + i] = image->GetScalarComponentAsFloat(i+xmin,j+ymin,0,0);
                green[j*xdim + i] = image->GetScalarComponentAsFloat(i+xmin,j+ymin,0,1);
                blue[j*xdim + i] = image->GetScalarComponentAsFloat(i+xmin,j+ymin,0,2);
                alpha[j*xdim + i] = image->GetScalarComponentAsFloat(i+xmin,j+ymin,0,3);
            }
        }
    }
    // otherwise, set alpha in each pixel to be maximum
    else if (image->GetNumberOfScalarComponents() == 3)
    {
        for(int j=0; j < ydim; j++)
        {
            for(i=0; i < xdim; i++)
            {
                red[j*xdim + i] = image->GetScalarComponentAsFloat(i+xmin,j+ymin,0,0);
                green[j*xdim + i] = image->GetScalarComponentAsFloat(i+xmin,j+ymin,0,1);
                blue[j*xdim + i] = image->GetScalarComponentAsFloat(i+xmin,j+ymin,0,2);
            }
        }
        for(i=0; i<xdim*ydim; i++)
            alpha[i] = 255.0;
    }
    else if (image->GetNumberOfScalarComponents() == 1)
    {
        for(int j=0; j < ydim; j++)
        {
            for(i=0; i < xdim; i++)
            {
                red[j*xdim + i] = green[j*xdim + i] = blue[j*xdim + i] =
                    image->GetScalarComponentAsFloat(i+xmin,j+ymin,0,0);
            }
        }
        for(i=0; i<xdim*ydim; i++)
            alpha[i] = 255.0;
    }
    else
    {
        EXCEPTION1(VisItException, "The image had an unexpected number of "
                   "components.");
    }

    // calculate intensity
    intensity.resize(xdim*ydim);
    for(i=0; i < xdim*ydim; i++)
        intensity[i] = (red[i] + green[i] + blue[i]) / 3.;

    // add these variables to the list
    cellvars.push_back(red);
    cellvars.push_back(green);
    cellvars.push_back(blue);
    cellvars.push_back(alpha);
    cellvars.push_back(intensity);

    readInImage = true;
#endif
}


// ****************************************************************************
//  Method: avtImageFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Chris Wojtan
//  Creation:   Thu Jun 3 09:50:31 PDT 2004
//
// ****************************************************************************

void
avtImageFileFormat::FreeUpResources(void)
{
    cellvars.clear();
    cellvarnames.clear();
    pointvars.clear();
    pointvarnames.clear();

    if( image != NULL)
    {
        image->Delete();
        image = NULL;
    }

    readInImage = false;
}


// ****************************************************************************
//  Method: avtImageFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Chris Wojtan
//  Creation:   Thu Jun 3 09:50:31 PDT 2004
//
//  Modifications:
//  
//     Chris Wojtan Mon Jun 21 15:39 PDT 2004
//     Moved variable name loading from constructor to this function,
//     Changed the 3D vector to 4D vector
//
// ****************************************************************************

void
avtImageFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    //if( !xdim || !ydim)
    //    EXCEPTION1(InvalidFilesException, fname.c_str());

    AddMeshToMetaData(md, "ImageMesh", AVT_RECTILINEAR_MESH, NULL, 1, 0, 2, 2);

    // load variable names
    cellvarnames.push_back("red");
    cellvarnames.push_back("green");
    cellvarnames.push_back("blue");
    cellvarnames.push_back("alpha");
    cellvarnames.push_back("intensity");

    // we do not create a list of vector variables,
    // because there is only one vector variable, "color", 
    // which is created from these scalar variables

    // add these variable names to the metadata
    int i;
    for(i=0; i<cellvarnames.size(); i++)
    {
        AddScalarVarToMetaData(md, cellvarnames[i], "ImageMesh", AVT_ZONECENT);
    }
    AddVectorVarToMetaData(md, "color", "ImageMesh", AVT_ZONECENT, 4);
}


// ****************************************************************************
//  Method: avtImageFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Chris Wojtan
//  Creation:   Thu Jun 3 09:50:31 PDT 2004
//
//  Modifications:
//    Mark C. Miller, Thu Nov  4 18:21:25 PST 2004
//    I made it get xdim/ydim (and mins/maxs) from vtkImageData object
//
// ****************************************************************************

vtkDataSet *
avtImageFileFormat::GetMesh(const char *meshname)
{
    if(!readInImage)
        ReadInImage();
    
    int dims[3];
    image->GetDimensions(dims);
    int xdim = dims[0];
    int ydim = dims[1];

    int extents[6];
    image->GetExtent(extents);
    int xmin = extents[0];
    int xmax = extents[1];
    int ymin = extents[2];
    int ymax = extents[3];

    // Set up rectilinear grid representing the image...
    // Since the colors are cell variables,
    //    we create an extra row of nodes in each dimesion
    //    so we can have the correct number of cells.
    int i;
    vtkFloatArray *xCoords = vtkFloatArray::New();
    for(i=0; i<xdim + 1; i++)
        xCoords->InsertNextValue((float) i+xmin);
    vtkFloatArray *yCoords = vtkFloatArray::New();
    for(i=0; i<ydim + 1; i++)
        yCoords->InsertNextValue((float) i+ymin);
    vtkFloatArray *zCoords = vtkFloatArray::New();
    zCoords->InsertNextValue(0.0);
    
    vtkRectilinearGrid *dataset = vtkRectilinearGrid::New();
    dataset->SetDimensions(xdim+1,ydim+1,1);
    dataset->SetXCoordinates(xCoords);
    dataset->SetYCoordinates(yCoords);
    dataset->SetZCoordinates(zCoords);

    xCoords->Delete();
    yCoords->Delete();
    zCoords->Delete();

    return dataset;
}


// ****************************************************************************
//  Method: avtImageFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Chris Wojtan
//  Creation:   Thu Jun 3 09:50:31 PDT 2004
//
//  Modifications:
//    Mark C. Miller, Thu Nov  4 18:21:25 PST 2004
//    I made it get xdim/ydim from vtkImageData object
// ****************************************************************************

vtkDataArray *
avtImageFileFormat::GetVar(const char *varname)
{
    if(!readInImage)
        ReadInImage();

    int dims[3];
    image->GetDimensions(dims);
    int xdim = dims[0];
    int ydim = dims[1];

    // Set up rectilinear grid representing the image...
    int i;
    for(i=0; i<pointvarnames.size(); i++)
    {
        if (pointvarnames[i] == string(varname))
        {

            vtkFloatArray *scalars = vtkFloatArray::New();
            scalars->SetNumberOfTuples(xdim*ydim);
            float *ptr = (float *)scalars->GetVoidPointer(0);
            memcpy(ptr, &pointvars[i][0], sizeof(float) * (xdim*ydim));
            return scalars;
        }
    }

    for(i=0; i<cellvarnames.size(); i++)
    {
        if (cellvarnames[i] == string(varname))
        {

            vtkFloatArray *scalars = vtkFloatArray::New();
            scalars->SetNumberOfTuples((xdim)*(ydim));
            float *ptr = (float *)scalars->GetVoidPointer(0);
            memcpy(ptr, &cellvars[i][0], sizeof(float) * (xdim*ydim));
            return scalars;
        }
    }

    EXCEPTION1(InvalidVariableException, varname);
}


// ****************************************************************************
//  Method: avtImageFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Chris Wojtan
//  Creation:   Thu Jun 3 09:50:31 PDT 2004
//
//  Modifications:
//      Chris Wojtan Mon Jun 21 15:41 PDT 2004
//      Changed the vector dimension from 3D to 4D
//
//      Mark C. Miller, Thu Nov  4 18:21:25 PST 2004
//      I made it get xdim/ydim from vtkImageData object
//
// ****************************************************************************


vtkDataArray *
avtImageFileFormat::GetVectorVar(const char *varname)
{
    if(!readInImage)
        ReadInImage();

    int dims[3];
    image->GetDimensions(dims);
    int xdim = dims[0];
    int ydim = dims[1];

    int ncomps = 4;
    int ntuples = xdim*ydim;   // this is the number of entries in the variable.
    vtkFloatArray *rv = vtkFloatArray::New();

    int ucomps = 4;

    rv->SetNumberOfComponents(ucomps);
    rv->SetNumberOfTuples(ntuples);
    float *one_entry = new float[ucomps];
    for (int i = 0 ; i < ntuples ; i++)
    {
        int j;
        for (j = 0 ; j < ncomps ; j++)
            one_entry[j] = cellvars[j][i];
        for (j = ncomps ; j < ucomps ; j++)
            one_entry[j] = 0.;
        rv->SetTuple(i, one_entry); 
    }
    
    delete [] one_entry;
    return rv;
}
